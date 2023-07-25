//#include <sys/mman.h>
//#include <cstddef>


// NOTE: The pool de-alloc function does not check whether the element was ever allocated. I am not
// sure how to do this - perhaps with an occupation list. However tis adds complexity tremendously.
// Maybe with a hash list, seems overkill. Having a no-use header for every element either messes
// up alignment or compactness.
// TODO: arena de-allocation that shrinks commited memory (useful when e.g. closing large files)
// TODO: scratch arenas
// TODO: be able to wrap a finite-sized arena around an array arg (could be a static array e.g.)



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

u64 MemoryProtect(void *from, u64 amount) {
    mprotect(from, amount, PROT_READ | PROT_WRITE);
    return amount;
}
void *MemoryReserve(u64 reserve_size) {
    void *result = mmap(NULL, ARENA_RESERVE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    return result;
}
MArena ArenaCreate() {
    MArena a;

    a.mem = (u8*) mmap(NULL, ARENA_RESERVE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    a.mapped = ARENA_RESERVE_SIZE;
    mprotect(a.mem, ARENA_COMMIT_CHUNK, PROT_READ | PROT_WRITE);
    a.committed = ARENA_COMMIT_CHUNK;
    a.used = 0;

    return a;
}
inline
void *ArenaAlloc(MArena *a, u64 len) {
    assert(!a->locked && "ArenaAlloc: memory arena is open, use MArenaClose to allocate");

    if (a->committed < a->used + len) {
        MemoryProtect(a->mem + a->committed, (len / ARENA_COMMIT_CHUNK + 1) * SIXTEEN_KB);
    }
    void *result = a->mem + a->used;
    a->used += len;

    return result;
}
void *ArenaPush(MArena *a, void *data, u32 len) {
    void *dest = ArenaAlloc(a, len);
    memcpy(dest, data, len);
    return dest;
}
void *ArenaOpen(MArena *a) {
    assert(!a->locked && "ArenaOpen: memory arena is alredy open");

    a->locked = true;
    return a->mem + a->used;
}
void ArenaClose(MArena *a, u64 len) {
    assert(a->locked && "ArenaClose: memory arena not open");

    a->locked = false;
    ArenaAlloc(a, len);
}


//
// Memory pool allocator / slot-based allocation with a free-list


struct MPool {
    u8 *mem;
    u32 block_size;
    u32 nblocks;
    LList1 *free_list;
};

#define MPOOL_CACHE_LINE_SIZE 64

MPool PoolCreate(u32 block_size_min, u32 nblocks) {
    assert(nblocks > 1);

    MPool p;
    p.block_size = MPOOL_CACHE_LINE_SIZE * (block_size_min / MPOOL_CACHE_LINE_SIZE + 1);
    p.mem = (u8*) mmap(NULL, p.block_size * nblocks, PROT_READ | PROT_WRITE, MAP_PRIVATE, -1, 0);
    p.free_list = (LList1*) p.mem;

    LList1 *current = p.free_list;
    for (u32 i = 0; i < nblocks - 1; ++i) {
        current->next = (LList1*) (p.mem + i * p.block_size);
        current = current->next;
    }
    current->next = NULL;

    return p;
}
void *PoolAlloc(MPool *p) {
    if (p->free_list == NULL) {
        return NULL;
    }
    void *retval = p->free_list;
    p->free_list = p->free_list->next;
    memzero(retval, MPOOL_CACHE_LINE_SIZE);

    return retval;
}
void PoolFree(MPool *p, void *element) {
    assert(element >= (void*) p->mem); // check lower bound
    u64 offset = (u8*) element -  p->mem;
    assert(offset % p->block_size == 0); // check alignment
    assert(offset < p->block_size * p->nblocks); // check upper bound

    LList1 *element_as_list = (LList1*) element;
    element_as_list->next = p->free_list;
    p->free_list = element_as_list;
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
    T* lst;
    ListX() {
        this->len = 0;
        this->arena = ArenaCreate();
        this->lst = (T*)arena.mem;
    }
    inline
    void Add(T el) {
        ArenaPush(&this->arena, &el, sizeof(T));
    }
    T* At(u32 idx) {
        return (T*)arena.mem + idx;
    }
};



// Wrapped list using void* and sizes:
//
// - wraps around existing pointer
// - separate constructor function for each simple types
/*
    u8 memarr[1024];
    List lst = ListCreate(memarr, sizeof(u32), 1024 / 4);
*/
struct List {
    void *mem;
    u32 element_sz;
    u32 len;
    u32 max_len;
};
List ListCreate(u8* mem, u32 element_sz, u32 max_len) {
    List lst;
    lst.mem = mem;
    lst.element_sz = element_sz;
    lst.len = 0;
    lst.max_len = max_len;
    return lst;
}
List ListCreateU8(u8 *mem, u32 max_len) { return ListCreate((u8*) mem, sizeof(u8), max_len); }
List ListCreateU16(u16 *mem, u32 max_len) { return ListCreate((u8*) mem, sizeof(u16), max_len); }
List ListCreateU32(u32 *mem, u32 max_len) { return ListCreate((u8*) mem, sizeof(u32), max_len); }
List ListCreateU364(u64 *mem, u32 max_len) { return ListCreate((u8*) mem, sizeof(u64), max_len); }
List ListCreateS16(s16 *mem, u32 max_len) { return ListCreate((u8*) mem, sizeof(s16), max_len); }
List ListCreateS32(s32 *mem, u32 max_len) { return ListCreate((u8*) mem, sizeof(s32), max_len); }
List ListCreateS364(s64 *mem, u32 max_len) { return ListCreate((u8*) mem, sizeof(s64), max_len); }



//
// C-style list: Array subscripting is done on the actual pointer, using the C native syntax.
// The pointer is associated with a len and a capacity/max_len
//
// NOTE: the macros below rely on implicit casts from void* (the return value of lst__grow), which
// is not allowed in C++. Can this style of list even work in C++ ?
// TODO: fix !


// Stretchy buffer

/*
u32 *mylst = NULL;
struct LstHdr {
    u32 len;
    u32 cap;
    u8* lst;
};

#define lst__hdr(lst)     ( lst ? ((LstHdr*) ( (u8*) lst - offsetof(LstHdr, lst) )) : 0 )
#define lst__fits(lst, n) ( lst_cap(lst) >= lst_len(lst) + n )
#define lst__fit(lst, n)  ( lst__fits(lst, n) ? 0 : lst = lst__grow(lst__hdr(lst), lst_len(lst) + n, sizeof(*lst)) )

#define lst_len(lst)      ( lst ? *((u32*)lst__hdr(lst)) : 0 )
#define lst_cap(lst)      ( lst ? *((u32*)lst__hdr(lst) + 1) : 0 )
#define lst_push(lst, e)  ( lst__fit(lst, 1), lst[lst_len(lst)++] = e )

void *lst__grow(LstHdr *hdr, u32 new_len, size_t e_sz) {
    u32 tot_len = new_len*e_sz + sizeof(LstHdr);
    if (hdr == NULL) {
        hdr = (LstHdr*) malloc(tot_len);
        hdr->lst = (u8*) hdr + offsetof(LstHdr, lst);
    }
    else {
        hdr->lst = (u8*) realloc(hdr, tot_len);
    }
    return hdr->lst;
}
*/


