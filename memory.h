#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstdlib>
#include <cstring>
#include <assert.h>

#include "types.h"


/**
* Linked list supporting mainly the GPA.
*/
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


/**
* General memory management, can manage memory blocks of different sizes.
* Use to keep a fixed memory footprint with full memory freedom.
*
* Note that the current linked-list based impl. is quite slow, when used with a
* significant amount of blocks/elements.
*/
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


/**
* Allocates and frees same-sized blocks of memory.
* Use to keep a fixed memory footprint with full memory freedom
* with equally (or similarly) sized objects.
*/
class PoolAllocator {
public:
    void* root;
    MemoryBlock* blocks;
    size_t block_size;
    size_t pool_size;
    size_t min_block_size;

    unsigned long load = 0;

    PoolAllocator(size_t block_size, size_t pool_size) {
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


/**
* Stack allocation w. reverse unwinding. Allocation pointer can be kept open-ended, unlock
* Stack by providing a size.
*/
// TODO: refactor into a struct.
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

void *_AllocStructVar(StackAllocator *stack, void *src, u32 size) {
    void* result = stack->Alloc(size);
    memcpy(result, src, size);
    return result;
}


/**
* ArrayList base functions.
*/
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


template<typename T>
struct List {
    T* lst = NULL;
    u32 len = 0;
    void Init(void* memloc) {
        this->lst = (T*) memloc;
    }
    void Add(T* item) {
        this->len++;
        ArrayPut(this->lst, sizeof(T), this->len, this->len - 1, item);
    }
    void Insert(T* item, u32 at_idx) {
        ArrayShift(this->lst, sizeof(T), this->len, at_idx, 1);
        this->len++;
        ArrayPut(this->lst, sizeof(T), this->len, at_idx, item);
    }
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
};


// TODO: Refactor into a header struct UnevenList<P,C> with a little state (root, len, iter) and 
//      methods functionality for iteration, e.g. InterStart(), IterNext(). Attempt to internalize
//      pointer calculations for generating an UnevenList.

template<typename P, typename C>
struct ForwardLinkedArray {
    P properties;
    int num_children = 0;
    ForwardLinkedArray *next;
    C *children;

    u32 InitInlineRequiresPadding(u32 size_bytes) {
        // empty guard
        if (size_bytes == 0) {
            num_children = 0;
            next = NULL;
            return 0;
        }

        // NOTE: input data must have an 8/16 byte spacing to make room for our "next" and "children" pointers
        assert(size_bytes > sizeof(ForwardLinkedArray) && "ForwardLinkedArray: Invalid size_bytes, below minimum");

        u32 num_objs = 0;
        u32 read_bytes = 0;
        ForwardLinkedArray *current = this;
        while (current != NULL) {
            ++num_objs;
            current->children = (C*) ((u8*) current + sizeof(ForwardLinkedArray));
            current->next = (ForwardLinkedArray *) ((u8*) current->children + current->num_children * sizeof(C));
            read_bytes += (u8*) current->next - (u8*) current;
            assert(read_bytes <= size_bytes && "ForwardLinkedArray: Data size corruption");
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
    void AttachTail(ForwardLinkedArray *tail) {
        if (this == NULL) {
            return;
        }
        ForwardLinkedArray *current = this;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = tail;
    }
    ForwardLinkedArray *CalculateNextPtr(u32 num_children) {
        u32 sz = sizeof(ForwardLinkedArray) + num_children * sizeof(C);
        return (ForwardLinkedArray *) ((u8*) this + sz);
    }
    C *CalculateChildrenPtr() {
        u32 sz = sizeof(ForwardLinkedArray);
        return (C*) ((u8*) this + sz);
    }
};


#endif
