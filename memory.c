//#include <sys/mman.h>


// NOTE: The pool de-alloc function does not check whether the element was ever allocated. I am not
// sure how to do this - perhaps with an occupation list. However tis adds complexity tremendously.
// Maybe with a hash list, seems overkill. Having a no-use header for every element either messes
// up alignment or compactness.
// TODO: arena de-allocation that shrinks commited memory (useful when e.g. closing large files)
// TODO: scratch arenas


struct MArena {
    u8 *mem;
    u64 mapped;
    u64 committed;
    u64 used;
    bool locked = false;
};

#define ARENA_RESERVE_SIZE GIGABYTE
#define ARENA_COMMIT_CHUNK SIXTEEN_KB

struct MPool {
    u8 *mem;
    u32 block_size;
    u32 nblocks;
    LList1 *free_list;
};

#define MPOOL_CACHE_LINE_SIZE 64

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
