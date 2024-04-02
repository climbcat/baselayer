#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstddef>
#include <cstdlib>
#include <cassert>


// NOTE: The pool de-alloc function does not check whether the element was ever allocated. I am not
// sure how to do this - perhaps with an occupation list. However tis adds complexity tremendously.
// Maybe with a hash list, seems overkill. Having a no-use header for every element either messes
// up alignment or compactness.
// TODO: arena de-allocation that shrinks commited memory (useful when e.g. closing large files)
// TODO: scratch arenas
// TODO: be able to wrap a finite-sized arena around an array arg (could be a static array e.g.)
// TODO: with arena_open/close, have a way to ensure enough commited space via some reserve param or such


//
// platform dependent impl.:


u64 MemoryProtect(void *from, u64 amount);
void *MemoryReserve(u64 amount);


//
// Memory Arena allocator


struct MArena {
    u8 *mem;
    u64 mapped;
    u64 committed;
    u64 used;
    bool locked = false;
};

#define ARENA_RESERVE_SIZE GIGABYTE
#define ARENA_COMMIT_CHUNK SIXTEEN_KB


MArena ArenaCreate() {
    MArena a;
    a.used = 0;

    a.mem = (u8*) MemoryReserve(ARENA_RESERVE_SIZE);
    a.mapped = ARENA_RESERVE_SIZE;

    MemoryProtect(a.mem, ARENA_COMMIT_CHUNK);
    a.committed = ARENA_COMMIT_CHUNK;

    return a;
}
inline
void *ArenaAlloc(MArena *a, u64 len, bool zerod = true) {
    assert(!a->locked && "ArenaAlloc: memory arena is open, use MArenaClose to allocate");

    if (a->committed < a->used + len) {
        u64 amount = (len / ARENA_COMMIT_CHUNK + 1) * ARENA_COMMIT_CHUNK;
        MemoryProtect(a->mem + a->committed, amount);
        a->committed += amount;
    }
    void *result = a->mem + a->used;
    a->used += len;
    if (zerod) {
        _memzero(result, len);
    }

    return result;
}
void *ArenaPush(MArena *a, void *data, u32 len) {
    void *dest = ArenaAlloc(a, len);
    _memcpy(dest, data, len);
    return dest;
}
void *ArenaOpen(MArena *a, u32 max_len = SIXTEEN_MB) {
    assert(!a->locked && "ArenaOpen: memory arena is alredy open");
    
    // ensure max_len bytes have been comitted
    u32 used = a->used;
    ArenaAlloc(a, max_len);
    a->used = used;

    // proceed
    a->locked = true;
    return a->mem + a->used;
}
void ArenaClose(MArena *a, u64 len) {
    assert(a->locked && "ArenaClose: memory arena not open");

    a->locked = false;
    ArenaAlloc(a, len, false);
}
void ArenaPrint(MArena *a) {
    printf("Arena mapped/committed/used: %lu %lu %lu\n", a->mapped, a->committed, a->used);
}
void ArenaClear(MArena *a) {
    a->used = 0;
}


//
// Memory pool allocator / slot based allocation impl. using a free-list
//

// NOTE: Pool indices, if used, are counted from 1, and the value 0 is reserved as the NULL equivalent.
// TODO: Write a proper, indexed pool that is typed

struct MPool {
    u8 *mem;
    u32 block_size;
    u32 nblocks;
    u32 occupancy;
    LList1 free_list;
};

#define MPOOL_MIN_BLOCK_SIZE 64
static MPool pool_zero;
MPool PoolCreate(u32 block_size_min, u32 nblocks) {
    assert(nblocks > 1);

    MPool p = pool_zero;;
    p.block_size = MPOOL_MIN_BLOCK_SIZE * (block_size_min / MPOOL_MIN_BLOCK_SIZE + 1);
    p.nblocks = nblocks;
    u32 size = p.block_size * p.nblocks;

    p.mem = (u8*) MemoryReserve(size);
    MemoryProtect(p.mem, size);

    LList1 *freeblck = &p.free_list;
    for (u32 i = 0; i < nblocks; ++i) {
        freeblck->next = (LList1*) (p.mem + i * p.block_size);
        freeblck = freeblck->next;
    }
    freeblck->next = NULL;

    return p;
}
void *PoolAlloc(MPool *p) {
    if (p->free_list.next == NULL) {
        return NULL;
    }
    void *retval = p->free_list.next;
    p->free_list.next = p->free_list.next->next;
    _memzero(retval, p->block_size);

    ++p->occupancy;
    return retval;
}
bool PoolCheckAddress(MPool *p, void *ptr) {
    if (ptr == NULL) {
        return false;
    }
    bool b1 = (ptr >= (void*) p->mem); // check lower bound
    if (b1 == false) {
        return false;
    }
    u64 offset = (u8*) ptr -  p->mem;
    bool b2 = (offset % p->block_size == 0); // check alignment
    bool b3 = (offset < p->block_size * p->nblocks); // check upper bound

    return b2 && b3;
}
u32 PoolAllocIdx(MPool *p) {
    assert(p->nblocks <= 2^16 && "indices will always fit within 16 bit limits");

    void *element = PoolAlloc(p);
    if (element == NULL) {
        return 0;
    }

    u32 idx = ((u8*) element - (u8*) p->mem) / p->block_size;
    assert(idx < p->nblocks && "block index must be positive and less and the number of blocks");
    return idx;
}
inline
u32 PoolPtr2Idx(MPool *p, void *ptr) {
    PoolCheckAddress(p, ptr);
    if (ptr == NULL) {
        return 0;
    }
    u32 idx = ((u8*) ptr - (u8*) p->mem) / p->block_size;
    return idx;
}
inline
void *PoolIdx2Ptr(MPool *p, u32 idx) {
    assert(idx < p->block_size);

    if (idx == 0) {
        return NULL;
    }
    void *ptr = (u8*) p->mem + idx * p->block_size;
    return ptr;
}
bool PoolFree(MPool *p, void *element, bool enable_strict_mode = true) {
    assert(PoolCheckAddress(p, element) && "input address aligned and in range");
    LList1 *e = (LList1*) element;

    // check element doesn't carry a valid free-list pointer
    bool is_first_free_element = p->free_list.next == e;
    bool target_valid = PoolCheckAddress(p, e->next);
    if (target_valid || is_first_free_element) {
        if (enable_strict_mode) {
            assert(target_valid == false && "trying to free an element probably on the free list");
        }
        else {
            return false;
        }
    }

    // free it
    e->next = p->free_list.next;
    p->free_list.next = e;
    --p->occupancy;

    return true;
}
bool PoolFreeIdx(MPool *p, u32 idx) {
    void * ptr = PoolIdx2Ptr(p, idx);
    return PoolFree(p, ptr);
}


//
// Static Arrays - List / Stack


template<typename T>
struct List {
    T *lst = NULL;
    u32 len = 0;

    inline
    void Add(T *element) {
        lst[len++] = *element;
    }
    inline
    T *Add(T element) {
        lst[len++] = element;
        return LastPtr();
    }
    inline
    void Push(T element) {
        lst[len++] = element;
    }
    inline
    T Pop() {
        return lst[--len];
    }
    inline
    T Last() {
        assert(len > 0);
        return lst[len - 1];
    }
    inline
    T *LastPtr() {
        assert(len > 0);
        return lst + len - 1;
    }
    inline
    T First() {
        assert(len > 0);
        return lst[0];
    }
    inline
    void Delete(u32 idx) {
        T swap = Last();
        len--;
        lst[len] = lst[idx];
        lst[idx] = swap;
    }
};
template<class T>
List<T> InitList(MArena *a, u32 count, bool zerod = true) {
    List<T> _lst;
    _lst.len = 0;
    _lst.lst = (T*) ArenaAlloc(a, sizeof(T) * count, zerod);
    return _lst;
}
template<class T>
List<T> InitListOpen(MArena *a, u32 max_cnt) {
    List<T> _lst;
    _lst.len = 0;
    _lst.lst = (T*) ArenaOpen(a, sizeof(T) * max_cnt);
    return _lst;
}
template<class T>
void InitListClose(MArena *a, u32 count) {
    ArenaClose(a, sizeof(T) * count);
}
template<class T>
void ArenaShedTail(MArena *a, List<T> lst, u32 diff_T) {
    assert(a->used >= diff_T * sizeof(T));
    assert(a->mem + a->used == (u8*) (lst.lst + lst.len + diff_T));

    a->mem -= diff_T * sizeof(T);
}


template<typename T>
struct Stack {
    T *lst = NULL;
    u32 len = 0;
    u32 cap = 0;

    inline
    void Push(T element) {
        lst[len++] = element;
    }
    inline
    T Pop() {
        if (len) {
            return lst[--len];
        }
        else {
            T defval;
            _memzero(&defval, sizeof(T));
            return defval;
        }
    }
};
template<class T>
Stack<T> InitStack(MArena *a, u32 cap) {
    Stack<T> stc;
    stc.lst = (T*) ArenaAlloc(a, sizeof(T) * cap, true);
    stc.len = 0;
    stc.cap = cap;
    return stc;
}
template<class T>
Stack<T> InitStackStatic(T *mem, u32 cap) {
    Stack<T> stc;
    stc.len = 0;
    stc.cap = cap;
    stc.lst = (T*) mem;
    return stc;
}


//
// Dynamic Arrays


// eXpanding list using Templates:
//
// - memory managed by internal arena
// - array subscripting done by accessing a pointer member
/*
    ListX<u32> lst;
*/
template<typename T>
struct ListX {
    MArena arena;
    u32 len;
    T *lst;
    ListX() {
        this->len = 0;
        this->arena = ArenaCreate();
        this->lst = (T*)arena.mem;
    }
    inline
    void Add(T el) {
        ArenaPush(&this->arena, &el, sizeof(T));
    }
    T *At(u32 idx) {
        return (T*)arena.mem + idx;
    }
};


//
// Stretchy buffer
//
// Subscripting on the native C pointer.
// The pointer is associated with a len and a capacity/max_len stored before the actual array.
// e.g.:
/*
    s32 *lst = NULL;
    lst_push(lst, 42);
    lst_push(lst, -15);

    for (int i = 0; i < lst_len(lst); ++i) {
        printf("%d\n", lst[i]);
    }
*/
// TODO: allow the virtual memory -style of growing, for pointer stability (and maybe performance)
// TODO: adopt "double internal storage space" convention

struct LstHdr {
    u32 len;
    u32 cap;
    u8 list[];
};

#define lst__hdr(lst)     ( lst ? ((LstHdr*) ( (u8*) lst - sizeof(LstHdr) )) : 0 )
#define lst__fits(lst, n) ( lst ? lst__cap(lst) >= lst_len(lst) + n : 0 )
#define lst__fit(lst, n)  ( lst__fits(lst, n) ? 0 : lst = lst__grow(lst, n) )
#define lst__cap(lst)     ( lst ? *((u32*)lst__hdr(lst) + 1) : 0 )

#define lst_len(lst)      ( lst ? *((u32*)lst__hdr(lst)) : 0 )
#define lst_push(lst, e)  ( lst__fit(lst, 1), lst[lst_len(lst)] = e, lst__hdr(lst)->len++ )
#define lst_free(lst)     ( free(lst__hdr(lst)) )
#define lst_print(lst)    ( lst ? printf("len: %u, cap: %u\n", lst__hdr(lst)->len, lst__hdr(lst)->cap) : 0 )

template<class T>
T *lst__grow(T *lst, u32 add_len) {

    u32 new_cap = MaxU32(2 * lst__cap(lst), MaxU32(add_len, 16));
    u32 new_size = new_cap * sizeof(T) + offsetof(LstHdr, list);

    LstHdr *new_hdr = NULL;
    if (lst == NULL) {
        new_hdr = (LstHdr*) malloc(new_size);
        new_hdr->len = 0;
    }
    else {
        new_hdr = (LstHdr*) realloc(lst__hdr(lst), new_size);
    }
    new_hdr->cap = new_cap;
    return (T*) new_hdr->list;
}

#endif
