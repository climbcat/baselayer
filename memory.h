#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstdlib>
#include <cstring>
#include <assert.h>

#include "types.h"


struct MemoryBlock {
    MemoryBlock* next;
    MemoryBlock* prev;
    u32 total_size;
    bool used = false; // TODO: experiment with bitflags here
};
void ListInsert(MemoryBlock* item, MemoryBlock* after) {
    item->next = after->next;
    item->prev = after;
    after->next->prev = item;
    after->next = item;
}
void ListInsertBefore(MemoryBlock* item, MemoryBlock* before) {
    item->next = before;
    item->prev = before->prev;
    before->prev->next = item;
    before->prev = item;
}
void ListRemove(MemoryBlock* item) {
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->prev = NULL;
    item->next = NULL;
}
void ListInit(MemoryBlock* first) {
    first->next = first;
    first->prev = first;
}
void ListPrintSizes(MemoryBlock* from) {
    MemoryBlock* itm = from->next;
    u32 i = 0;
    while (itm != from) {
        printf("element #%u size=%d", i, itm->total_size);
        itm = itm->next;
        ++i;
    }
}
u32 ListLen(MemoryBlock* from) {
    MemoryBlock* itm = from->next;
    u64 i = 0;
    while (itm != from) {
        ++i;
        itm = itm->next;
    }
    return i;
}


class GeneralPurposeAllocator {
public:
    MemoryBlock* blocks;
    u32 max_size;
    u32 min_block_size;
    u32 num_blocks;
    u32 header_size;

    u32 blocks_merged = 0;
    u32 load = 0;

    GeneralPurposeAllocator(u32 max_size) {
        this->max_size = max_size;
        this->header_size = sizeof(MemoryBlock);
        this->min_block_size = 2 * this->header_size;

        this->blocks = (MemoryBlock*) calloc(max_size, 1);

        this->blocks->total_size = this->max_size;
        this->blocks->used = false;
        ListInit(this->blocks);
        this->num_blocks = 1;
    }
    ~GeneralPurposeAllocator() {
        std::free(this->blocks);
    }

    void* Alloc(u32 size) {
        const u32 alloc_stride = (u32) size / this->header_size + 2;
        const u32 alloc_size = alloc_stride * this->header_size;

        if (alloc_size > this->max_size - this->load)
            // TODO: out of memory / eviction strategy
            return NULL;

        MemoryBlock* at = this->blocks;
        u32 idx = 0;
        u32 free_size;
        while (idx < this->num_blocks) {

            if (ListLen(at) == 21) {
                for (int j = 0; j < 21; j++)
                  at = at->next;
            }

            if (at->used == false) {
                free_size = at->total_size;
                if (free_size >= alloc_size) {

                    if (free_size > alloc_size + this->min_block_size) {
                        MemoryBlock* remainder = at + alloc_stride;
                        remainder->total_size = free_size - alloc_size;
                        remainder->used = false;
                        ListInsert(remainder, at);
                        this->num_blocks++;
                    }

                    at->total_size = alloc_size; // TODO: fix, this is not always correct
                    at->used = true;

                    this->load += at->total_size;
                    return at + 1;
                }
            }

            at = at->next;
            ++idx;
        }

        assert(1 == 0);
    }

    bool _TryMergeAdjacentBlocks(MemoryBlock* a, MemoryBlock* b) {
        if (a->used == true || b->used == true || b == this->blocks)
            return false;

        MemoryBlock* tmp;
        if (b->next == a) {
            tmp = a;
            a = b;
            b = tmp;
        }
        else if (a->next != b)
            return false;

        // a sits right before b
        a->total_size += b->total_size;

        ListRemove(b);
        this->num_blocks--;
        this->blocks_merged++;
        return true;
    }

    bool Free(void* addr) {
        // guards
        size_t relative_location = (u8*) addr - (u8*) this->blocks;
        assert(relative_location >= 0);
        assert(relative_location < this->max_size);

        MemoryBlock* at = (MemoryBlock*) (((u8*) addr) - sizeof(MemoryBlock));
        if (at->used == false)
            return false;

        assert(this->load >= at->total_size);

        this->load -= at->total_size;
        at->used = false;

        this->_TryMergeAdjacentBlocks(at->prev, at);

        return true;
    }
};


// TODO: Write templated struct versions of PoolAllocor and StackAllocator.


class PoolAllocator {
public:
    void* root;
    MemoryBlock* blocks;
    u32 block_size;
    u32 pool_size;
    u32 min_block_size;

    unsigned long load = 0;

    PoolAllocator(u32 block_size, u32 pool_size) {
        this->min_block_size = sizeof(MemoryBlock);
        assert(block_size >= this->min_block_size);

        this->block_size = block_size;
        this->pool_size = pool_size;

        this->blocks = (MemoryBlock*) calloc(block_size * pool_size, 1);
        this->blocks->used = false;
        this->blocks->total_size = this->block_size;
        ListInit(this->blocks);

        // record for destructor
        this->root = this->blocks;

        // build the free list
        MemoryBlock* prev = this->blocks;
        MemoryBlock* next;
        for (int i = 1; i < this->pool_size; i++) {
            next = (MemoryBlock*) ((u8*) this->blocks + (i * this->block_size));
            next->total_size = this->block_size;
            next->used = false;

            ListInsert(next, prev);
            prev = next;
        }
    }
    ~PoolAllocator() {
        free(this->root);
    }

    void* Get() {
        if (this->blocks == NULL) {
            assert(this->load == this->pool_size);
            return NULL;
        }

        this->load++;
        MemoryBlock* toalloc = this->blocks;
        if (this->blocks->next != this->blocks) {
            this->blocks = toalloc->next;
            ListRemove(toalloc);
        }
        else {
            // list is now full
            assert(this->load == this->pool_size);
            this->blocks = NULL;
        }

        memset(toalloc, sizeof(MemoryBlock), 0);
        return toalloc;
    }

    bool Release(void* addr) {
        size_t relative_location = (u8*) addr - (u8*) this->root;
        assert(relative_location >= 0);
        assert(relative_location < this->pool_size * this->block_size);
        assert(relative_location % this->block_size == 0); // alignment

        if (this-load == 0)
            return false;

        MemoryBlock* addrblock = (MemoryBlock*) addr;
        addrblock->total_size = this->block_size;
        addrblock->used = false;

        if (this->blocks != NULL) {
            ListInsertBefore(addrblock, this->blocks);
        }
        else {
            // first free element after full
            assert(this->load == this->pool_size);
            this->blocks = (MemoryBlock*) addr;
            ListInit(this->blocks);
        }

        this->load--;
        return true;
    }
};


class StackAllocator {
public:
    void* root = NULL;
    u32 max_size = 0;
    u32 num_blocks = 0;
    u32 used = 0;
    bool locked = false;

    StackAllocator(u32 total_size) {
        this->max_size = total_size;
        this->used = 0;
        this->root = malloc(this->max_size);
    }
    ~StackAllocator() {
        free(this->root);
    }
    void Clear() {
        this->used = 0;
        this->num_blocks = 0;
    }
    void* Alloc(u32 size) {
        assert(this->locked == false);
        assert(size <= this->max_size - this->used && "checking stack size limit");

        void* retval = (u8*) this->root + this->used;
        if (size > 0) {
            this->used += size;
            this->num_blocks++;
        }
        return retval;
    }
    void* AllocOpenEnded() {
        assert(this->locked == false);

        this->locked = true;
        return (u8*) this->root + this->used;
    }
    void CloseOpenEnded(u32 final_size) {
        assert(this->locked == true);

        this->locked = false;
        this->Alloc(final_size);
    }
    void CancelOpenEnded() {
        this->locked = false;
    }
    bool Free(void* addr) {
        u32 relative_location = (u8*) addr - (u8*) this->root;
        assert(relative_location >= 0);
        assert(relative_location < this->max_size);

        if (relative_location >= this->used)
        return false;

        memset((u8*) this->root + relative_location, this->used - relative_location, 0);

        this->used = relative_location;
        this->num_blocks--;
        return true;
    }
};

char *AllocConstString(const char* word, StackAllocator *stack) {
    char *dest = (char*) stack->Alloc(strlen(word) + 1);
    strcpy(dest, word);
    return dest;
}

char *AllocConstStringPrefixed(char* prefix, const char* word, StackAllocator *stack) {
    char *dest = (char*) stack->Alloc(strlen(prefix) + strlen(word) + 1);
    strcpy(dest, prefix);
    strcat(dest, word);
    return dest;
}

char *AllocConstStringAffixed(const char* word, char* affix, StackAllocator *stack) {
    char *dest = (char*) stack->Alloc(strlen(word) + strlen(affix) + 1);
    strcpy(dest, word);
    strcat(dest, affix);
    return dest;
}

void *AllocStructVar(StackAllocator *stack, void *src, u32 size) {
    void* result = stack->Alloc(size);
    memcpy(result, src, size);
    return result;
}


inline
void ArrayPut(void* lst, u32 element_size, u32 array_len, u32 at_idx, void* item) {
    assert(at_idx <= array_len);

    u8* dest = (u8*) lst + at_idx * element_size;
    memcpy(dest, item, element_size);
}

inline
void ArrayShift(void* lst, int element_size, u32 array_len, int at_idx, int offset) {
    assert(at_idx >= 0);

    u8* src = (u8*) lst + at_idx * element_size;
    u8* dest = (u8*) lst + (at_idx + offset) * element_size;
    if (at_idx + offset < 0) {
        dest = (u8*) lst;
        src = (u8*) lst + abs(offset) * element_size;
    }

    assert(src - (u8*) lst >= 0);
    assert(dest - (u8*) lst >= 0);
    assert(src - (u8*) lst <= array_len * element_size);
    assert(dest - (u8*) lst < array_len * element_size);
    assert(array_len - at_idx >= 0);

    memmove(dest, src, (array_len - at_idx) * element_size);
}

// TODO: Add a max_len parameter and member and checks to not exceed it during Add, Insert and At
template<typename T>
struct List {
    T* lst = NULL;
    u32 len = 0;
    void Init(void* memloc) {
        this->lst = (T*) memloc;
        this->len = 0;
    }
    T* Add(T* item) {
        this->len++;
        ArrayPut(this->lst, sizeof(T), this->len, this->len - 1, item);
        return At(this->len - 1);
    }
    void Insert(T* item, u32 at_idx) {
        // TODO: make sure the last element is not mem-cpd outside of the range of len.
        ArrayShift(this->lst, sizeof(T), this->len, at_idx, 1);
        this->len++;
        ArrayPut(this->lst, sizeof(T), this->len, at_idx, item);
    }
    // TODO: should be a non-order preserving rm, whiches out the last entry
    void Remove(u32 at_idx) {
        ArrayShift(this->lst, sizeof(T), this->len, at_idx + 1, -1);
        this->len--;
    }
    List<T> Collapse(List<T> tail) {
        assert( &lst[len + 1] == &tail.lst[0] && "tail must be located precisely after head/this");
        List<T> result { lst, len + tail.len };
        return result;
    }
    T* At(u32 idx) {
        return this->lst + idx;
    }
    // TODO: Creat a "New" function, which gives a point to a new, correctly initialized entry, and incs the list.
};


template<typename T>
struct Stack {
    T* lst = NULL;
    u32 len = 0;
    void Init(void* memloc) {
        this->lst = (T*) memloc;
    }
    void Push(T* item) {
        this->len++;
        ArrayPut(this->lst, sizeof(T), this->len, this->len - 1, item);
    }
    T Pop() {
        if (this->len == 0) {
            return NULL;
        }
        this->len--;
        return *(this->lst + this->len);
    }
};


template<typename T>
struct TreeIterDF {
    T *_stack[100];
    Stack<T*> stack { &_stack[0], 0 };
    void Init(T *root, bool iter_root = true) {
        if (root->enabled == false) {
            return;
        }
        if (iter_root == true) {
            this->stack.Push(&root);
        }
        else {
            for (int i = 0; i < root->children.len; ++i) {
                T *element = root->children.lst[i];
                if (element->enabled == true) {
                    this->stack.Push(root->children.lst + i);
                }
            }
        }
    }
    T *Next() {
        T *current = this->stack.Pop();
        if (current == NULL) {
            return NULL;
        }
        for (int i = 0; i < current->children.len; ++i) {
            T *element = current->children.lst[i];
            if (element->enabled == true) {
                this->stack.Push(current->children.lst + i);
            }
        }
        return current;
    }
};


template<typename T>
struct LinkedList {
    bool root = false;
    LinkedList *next = NULL;
    LinkedList *prev = NULL;
    T element;

    void Init() {
        root = true;
        next = this;
        prev = this;
    }
    bool Insert(LinkedList *l) {
        if (l == NULL) {
            return false;
        }
        l->prev = this;
        l->next = next;
        next->prev = l;
        next = l;
        return true;
    }
    bool Remove() {
        if (this->root == true) {
            return false;
        }
        else {
            prev->next = next;
            next->prev = prev;
            return true;
        }
    }
    LinkedList *Iter() {
        if (next != NULL && next->root == true) {
            return NULL;
        }
        else {
            return next;
        }
    }
    bool IsEmpty() {
        assert((this->next == this && this->prev == this) || (this->next != this && this->prev != this) && "check ll local integrity");
        return this->next == this;
    }
};


template<typename P, typename C>
struct UnevenList {
    P properties;
    int num_children = 0;
    UnevenList *next;
    C *children;

    u32 InitInlineRequiresPadding(u32 size_bytes) {
        if (size_bytes == 0) {
            num_children = 0;
            next = NULL;
            return 0;
        }

        assert(size_bytes >= sizeof(UnevenList) && "UnevenList: Invalid size_bytes, below minimum (input data must have an 8/16 byte spacing to make room for our next and children pointers)");

        u32 num_objs = 0;
        u32 read_bytes = 0;
        UnevenList *current = this;
        while (current != NULL) {
            ++num_objs;
            current->children = (C*) ((u8*) current + sizeof(UnevenList<P,C>));
            current->next = (UnevenList *) (current->children + current->num_children);

            read_bytes += (u8*) current->next - (u8*) current;

            assert(read_bytes <= size_bytes && "UnevenList: Data size corruption");
            if (read_bytes == size_bytes) {
                current->next = NULL;
                current = NULL;
            }
            else {
                current = current->next;
            }
        }
        return num_objs;
    }
    void AttachTail(UnevenList *tail) {
        if (this == NULL) {
            return;
        }
        UnevenList *current = this;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = tail;
    }
    UnevenList *CalculateNextPtr(u32 num_children) {
        u32 sz = sizeof(UnevenList) + num_children * sizeof(C);
        return (UnevenList *) ((u8*) this + sz);
    }
        C *CalculateChildrenPtr() {
        u32 sz = sizeof(UnevenList);
        return (C*) ((u8*) this + sz);
    }
};


// TODO: Internalize pointer calculations for UnevenList generation. Think of this object as the equivalent of the List<T> struct.
template<typename P, typename C>
struct UnevenListHeader {
    UnevenList<P,C> *root;
    u32 max_size = 0;
    u32 len = 0;
    u32 iter = 0;
    void Init(u8* memloc, u32 max_size) {
        max_size = max_size;
    }
    UnevenList<P,C> *IterStart() {
        // TODO: impl.
    }
    UnevenList<P,C> *IterNext() {
        // TODO: impl.
    }
};


//
// Ring buffer: Lock-less, single-reader, single-writer thread safe.


#include <atomic>
#include <condition_variable>
#include <cstring>

template <typename T>
struct LLRingBuffer
{
    T *buffer;
    size_t _el_size;
    std::atomic<int> pos_in = 1;
    std::atomic<int> pos_out = 1;
    std::atomic_flag lread1;
    std::atomic_flag lwrite1;

    unsigned int overflow = 0;
    int size;

    void Init(int size, int element_size = -1)
    {
        this->pos_in = 1;
        this->pos_out = 1;

        this->size = size;
        this->_el_size = sizeof(T);
        if (element_size > 0)
            this->_el_size = element_size;
        this->buffer = (T *)malloc((this->_el_size + this->_customsz_num_bytes) * this->size);

        lwrite1.clear(std::memory_order_release);
        lread1.clear(std::memory_order_release);
    }

    bool _push_conflict()
    {
        return (pos_in == pos_out - 1) || (pos_in == size - 1 && pos_out == 0);
    }
    void _inc(std::atomic<int> *pos)
    {
        (*pos)++;
        if (*pos % size == 0)
            *pos = 0;
    }

    void put(T *src, int copy_size = 0)
    {
        while (lwrite1.test_and_set(std::memory_order_acquire))
            ;

        uint8_t *buffer_loc = (uint8_t *) this->buffer + this->pos_in * this->_el_size;
        if (copy_size <= 0 || copy_size > this->_el_size)
            copy_size = this->_el_size;
        memcpy(buffer_loc + this->_customsz_num_bytes, src, copy_size);

        if (_push_conflict())
        {
            // conflict: incrementing pos_in would result in pos_in == pos_out
            while (lread1.test_and_set(std::memory_order_acquire))
                ;
            // buffer full, discard old
            _inc(&pos_out);
            ++overflow;
            lread1.clear(std::memory_order_release);
        }

        _inc(&pos_in);
        lwrite1.clear(std::memory_order_release);
    }

    bool get(T *dest, int *size_out = nullptr)
    {
        while (lread1.test_and_set(std::memory_order_acquire))
            ;
        if (pos_out == pos_in)
        {
            // queue was empty
            lread1.clear(std::memory_order_release);
            return false;
        }
        // since lread1 is set, we may assume that pos_out has not been incremented

        uint8_t *buffer_loc = (uint8_t *)this->buffer + this->pos_out * this->_el_size;
        size_t copy_size = this->_el_size;
        memcpy(dest, buffer_loc, copy_size);

        _inc(&pos_out);

        lread1.clear(std::memory_order_release);
        return true;
    }

    inline bool empty()
    {
        return (pos_in == pos_out);
    }

    int current_load()
    {
        int diff = this->pos_in - this->pos_out;
        if (diff >= 0) 
        {
            return diff;
        }
        else
        {
            return this->size + diff;
        }
    }
};


#endif
