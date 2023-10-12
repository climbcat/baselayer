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
void *ArenaAlloc(MArena *a, u64 len) {
    assert(!a->locked && "ArenaAlloc: memory arena is open, use MArenaClose to allocate");

    if (a->committed < a->used + len) {
        u64 amount = (len / ARENA_COMMIT_CHUNK + 1) * ARENA_COMMIT_CHUNK;
        MemoryProtect(a->mem + a->committed, amount);
        a->committed += amount;
    }
    void *result = a->mem + a->used;
    a->used += len;

    return result;
}
void *ArenaPush(MArena *a, void *data, u32 len) {
    void *dest = ArenaAlloc(a, len);
    _memcpy(dest, data, len);
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
// Memory pool allocator / slot based allocation with a free-list


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
    p.mem = (u8*) MemoryProtect(NULL, p.block_size * nblocks);  // mmap(NULL, p.block_size * nblocks, PROT_READ | PROT_WRITE, MAP_PRIVATE, -1, 0);
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
    _memzero(retval, MPOOL_CACHE_LINE_SIZE);

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
