/*
MIT License

Copyright (c) 2025 Jakob Garde

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef __JG_BASELAYER_H__
#define __JG_BASELAYER_H__


#define BASELAYER_VERSION_MAJOR 0
#define BASELAYER_VERSION_MINOR 1
#define BASELAYER_VERSION_PATCH 1


//
//  base.h
//


#pragma warning(push)
#pragma warning(disable : 4200)
#pragma warning(disable : 4477)
#pragma warning(disable : 4996)
#pragma warning(disable : 4530)

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>

//
// basics


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;


#ifdef __arm__
#warning "WARN: __arm__ detected: u64 typedef'd to 32bit"
typedef uint32_t u64;
#else
typedef uint64_t u64;
#endif

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#define KILOBYTE 1024
#define SIXTEEN_KB (16 * 1024)
#define THIRTYTWO_KB (32 * 1024)
#define SIXTYFOUR_KB (64 * 1024)
#define MEGABYTE (1024 * 1024)
#define SIXTEEN_MB (16 * 1024 * 1024)
#define GIGABYTE (1024 * 1024 * 1024)
#define FOUR_GB (4 * 1024 * 1024 * 1024)


#define PI 3.14159f
f32 deg2rad = PI / 180.0f;
f32 rad2deg = 180.0f / PI;


inline u8 MinU8(u8 a, u8 b) { return (a <= b) ? a : b; }
inline u16 MinU16(u16 a, u16 b) { return (a <= b) ? a : b; }
inline u32 MinU32(u32 a, u32 b) { return (a <= b) ? a : b; }
inline u64 MinU64(u64 a, u64 b) { return (a <= b) ? a : b; }

inline s8 MinS8(s8 a, s8 b) { return (a <= b) ? a : b; }
inline s16 MinS16(s16 a, s16 b) { return (a <= b) ? a : b; }
inline s32 MinS32(s32 a, s32 b) { return (a <= b) ? a : b; }
inline s64 MinS64(s64 a, s64 b) { return (a <= b) ? a : b; }

inline f32 MinF32(f32 a, f32 b) { return (a <= b) ? a : b; }
inline f64 MinF64(f64 a, f64 b) { return (a <= b) ? a : b; }

inline u8 MaxU8(u8 a, u8 b) { return (a > b) ? a : b; }
inline u16 MaxU16(u16 a, u16 b) { return (a > b) ? a : b; }
inline u32 MaxU32(u32 a, u32 b) { return (a > b) ? a : b; }
inline u64 MaxU64(u64 a, u64 b) { return (a > b) ? a : b; }

inline s8 MaxS8(s8 a, s8 b) { return (a > b) ? a : b; }
inline s16 MaxS16(s16 a, s16 b) { return (a > b) ? a : b; }
inline s32 MaxS32(s32 a, s32 b) { return (a > b) ? a : b; }
inline s64 MaxS64(s64 a, s64 b) { return (a > b) ? a : b; }

inline f32 MaxF32(f32 a, f32 b) { return (a > b) ? a : b; }
inline f64 MaxF64(f64 a, f64 b) { return (a > b) ? a : b; }


//
// qol


inline void _memcpy(void *dest, const void *src, size_t size) {
    memcpy(dest, src, size);
    /*
    u8 *s = (u8*) src;
    u8 *d = (u8*) dest;

    for (u32 i = 0; i < size; ++i) {
        d[i] = s[i];
    }
    */
}
inline u32 _strcmp(const char *dest, const char *src) {
    u32 i = 0;
    while (dest[i] != '\0' || src[i] != '\0') {
        if (dest[i] != src[i]) {
            return 1;
        }
        ++i;
    }
    return 0;
}
inline u32 _strlen(char *str) {
    u32 i = 0;
    while (str[i] != '\0') {
        ++i;
    }
    return i;
}
inline void _memzero(void *dest, size_t n) {
    u8 *d = (u8*) dest;
    for (u32 i = 0; i < n; ++i) {
        d[i] = 0;
    }
}


//
// linked list


struct LList1 {
    LList1 *next;
};

struct LList2 {
    LList2 *next;
    LList2 *prev;
};

struct LList3 {
    LList3 *next;
    LList3 *prev;
    LList3 *descend;
};

void *InsertBefore1(void *newlink, void *before) {
    LList1 *newlnk = (LList1*) newlink;

    newlnk->next = (LList1*) before;
    return newlink;
}
void *InsertAfter1(void *after, void *newlink) {
    ((LList1*) after)->next = (LList1*) newlink;
    return newlink;
}
void InsertBefore2(void *newlink, void *before) {
    LList2 *newlnk = (LList2*) newlink;
    LList2 *befre = (LList2*) before;

    newlnk->prev = befre->prev;
    newlnk->next = befre;
    if (befre->prev != NULL) {
        befre->prev->next = newlnk;
    }
    befre->prev = newlnk;
}
void InsertBelow3(void *newlink, void *below) {
    LList3 *newlnk = (LList3*) newlink;
    LList3 *belw = (LList3*) below;

    newlnk->descend = belw->descend;
    belw->descend = newlnk;
}


//
// string conversion


u32 ParseInt(char *text) {
    u32 val = 0;
    u32 multiplier = 1;

    // signed?
    bool sgned = text[0] == '-';
    if (sgned) {
        ++text;
    }
    u32 len = _strlen(text);

    // decimals before dot
    for (u32 i = 0; i < len; ++i) {
        val += (text[len - 1 - i] - 48) * multiplier;
        multiplier *= 10;
    }

    // handle the sign
    if (sgned) {
        val *= -1;
    }

    return val;
}
u32 ParseInt(char *text, u32 len) {
    u32 val = 0;
    u32 multiplier = 1;

    // signed?
    bool sgned = text[0] == '-';
    if (sgned) {
        ++text;
    }

    // decimals before dot
    for (u32 i = 0; i < len; ++i) {
        val += (text[len - 1 - i] - 48) * multiplier;
        multiplier *= 10;
    }

    // handle the sign
    if (sgned) {
        val *= -1;
    }

    return val;
}


f64 ParseDouble(char *str, u8 len) {
    f64 val = 0;
    f64 multiplier = 1;

    // handle sign
    bool sgned = str[0] == '-';
    if (sgned) {
        ++str;
        --len;
    }
    if (len == 0) {
        return 0.0;
    }

    u8 decs_denom = 0;
    while ((str[decs_denom] != '.') && (decs_denom < len)) {
        ++decs_denom;
    }

    // decimals before dot
    for (int i = 0; i < decs_denom; ++i) {
        char ascii = str[decs_denom - 1 - i];
        u8 decimal = ascii - 48;
        val += decimal * multiplier;
        multiplier *= 10;
    }

    // decimals after dot
    multiplier = 0.1f;
    u8 decs_nom = len - 1 - decs_denom;
    for (int i = decs_denom + 1; i < len; ++i) {
        char ascii = str[i];
        u8 decimal = ascii - 48;
        val += decimal * multiplier;
        multiplier *= 0.1;
    }

    // handle the sign
    if (sgned) {
        val *= -1;
    }

    return val;
}


//
// cmd-line args


s32 g_argc;
char **g_argv;
void CLAInit(s32 argc, char **argv) {
    g_argc = argc;
    g_argv = argv;
}

bool CLAContainsArg(const char *search, int argc, char **argv, int *idx = NULL) {
    for (int i = 0; i < argc; ++i) {
        char *arg = argv[i];
        if (!_strcmp(argv[i], search)) {
            if (idx != NULL) {
                *idx = i;
            }
            return true;
        }
    }
    return false;
}

bool CLAContainsArgs(const char *search_a, const char *search_b, int argc, char **argv) {
    bool found_a = false;
    bool found_b = false;
    for (int i = 0; i < argc; ++i) {
        if (!_strcmp(argv[i], search_a)) {
            found_a = true;
        }
        if (!_strcmp(argv[i], search_b)) {
            found_b = true;
        }
    }
    return found_a && found_b;
}

char *CLAGetArgValue(const char *key, int argc, char **argv) {
    int i;
    bool error = !CLAContainsArg(key, argc, argv, &i) || i == argc - 1;;
    if (error == false) {
        char *val = argv[i+1];
        error = _strlen(val) > 1 && val[0] == '-' && val[1] == '-';
    }
    if (error == true) {
        printf("KW arg %s must be followed by a value arg\n", key);
        return NULL;
    }
    return argv[i+1];
}

char *CLAGetFirstArg(int argc, char **argv) {
    if (argc <= 1) {
        return NULL;
    }
    return argv[1];
}


//
//  profile.h
//

#define PROFILE 1
#ifndef PROFILE
#define PROFILE 0
#endif


u64 ReadSystemTimerMySec();
u32 ReadSystemTimerMySec32();
u64 ReadCPUTimer();
void XSleep(u32 ms);


#if PROFILE == 1 // enable profiler

struct ProfilerBlock {
    u64 elapsed_atroot_tsc;
    u64 elapsed_tsc;
    u64 elapsed_children_tsc;
    const char *tag;
    u32 hits;
};
struct Profiler {
    u64 total_tsc;
    u64 total_systime;
    float cpu_freq;
    u32 count;
    u32 active;
    ProfilerBlock blocks[1024];
};
void ProfilerStart(Profiler *p) {
    p->total_systime = ReadSystemTimerMySec();
    p->total_tsc = ReadCPUTimer();
}
void ProfilerStop(Profiler *p) {
    p->total_systime = ReadSystemTimerMySec() - p->total_systime;
    p->total_tsc = ReadCPUTimer() - p->total_tsc;
    p->cpu_freq = (float) p->total_tsc / p->total_systime;
}
float ProfilerGetCPUFreq(u32 measured_over_interval_ms) {
    u64 sys_start = ReadSystemTimerMySec();
    u64 tsc_start = ReadCPUTimer();
    XSleep(measured_over_interval_ms);
    u64 sys_end = ReadSystemTimerMySec();
    u64 tsc_end = ReadCPUTimer();
    float frequency = (float) (tsc_end - tsc_start) / (sys_end - sys_start);
    return frequency;
}
void ProfilerPrint(Profiler *p) {
    printf("\n");
    printf("Total time: %lu ms", p->total_systime / 1000);
    printf(" (tsc: %lu,", p->total_tsc);
    printf(" freq [tsc/mys]: %f)\n", p->cpu_freq);

    float pct_sum = 0;
    ProfilerBlock *current;
    for (u32 i = 1; i < p->count + 1; ++i) {
        current = p->blocks + i;
        printf("  %s: ", current->tag);

        u64 self_tsc = current->elapsed_tsc - current->elapsed_children_tsc;
        u64 self_ms = (u64) ((f32) self_tsc / p->cpu_freq / 1000.0f);
        f32 self_pct = (f32) self_tsc / p->total_tsc * 100;
        u64 total_tsc = current->elapsed_atroot_tsc;
        f32 total_pct = (f32) total_tsc / p->total_tsc * 100;

        u32 hits = current->hits;
        printf("%lu tsc %lu ms (%.2f%%) self, %lu (%.2f%%) tot %u hits)\n", self_tsc, self_ms, self_pct, total_tsc, total_pct, hits);
    }
}


class ProfileInitAndPrintMechanism {
public:
    Profiler *p;
    ProfileInitAndPrintMechanism(Profiler *p) {
        this->p = p;
        ProfilerStart(p);
    }
    ~ProfileInitAndPrintMechanism() {
        ProfilerStop(p);
        ProfilerPrint(p);
    }
};
class ProfileScopeMechanism {
public:
    Profiler *p;
    u32 slot;
    u64 start;
    u64 elapsed_atroot;
    u32 parent;
    ProfileScopeMechanism(Profiler *p, const char *tag, u32 slot) {
        this->p = p;

        this->start = ReadCPUTimer();
        this->elapsed_atroot = this->p->blocks[slot].elapsed_atroot_tsc;
        this->slot = slot;
        this->parent = this->p->active;

        this->p->blocks[slot].tag = tag;
        this->p->blocks[slot].hits += 1;

        this->p->count = MaxU32(this->p->count, slot);
        this->p->active = slot;
    }
    ~ProfileScopeMechanism() {
        u64 diff = ReadCPUTimer() - this->start;
        this->p->blocks[this->slot].elapsed_tsc += diff;
        this->p->blocks[this->slot].elapsed_atroot_tsc = this->elapsed_atroot + diff;
        
        if (this->parent) {
            this->p->blocks[this->parent].elapsed_children_tsc += diff;
        }
        this->p->active = parent;
    }
};


static Profiler g_prof;
#define TimeProgram ProfileInitAndPrintMechanism __prof_init__(&g_prof);
#define TimeFunction ProfileScopeMechanism __prof_mechanism__(&g_prof, __FUNCTION__, __COUNTER__ + 1);
#define TimeBlock(tag) ProfileScopeMechanism __prof_mechanism__(&g_prof, "<" tag ">", __COUNTER__ + 1);


#else // disable profiler, empty macros

// TODO: retain timing the entire program run as a very light-weight thing

#define TimeProgram ;
#define TimeFunction ;
#define TimeBlock(tag) ;
#endif


//
//  memory.h
//


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
s32 MemoryUnmap(void *at, u64 amount_reserved);


//
// Memory Arena allocator


struct MArena {
    u8 *mem;
    u64 mapped;
    u64 committed;
    u64 used;
    u64 fixed_size;
};

#define ARENA_RESERVE_SIZE GIGABYTE
#define ARENA_COMMIT_CHUNK SIXTEEN_KB


MArena ArenaCreate(u64 fixed_size = 0) {
    MArena a = {};
    a.used = 0;

    if (fixed_size > 0) {
        a.mem = (u8*) MemoryReserve(fixed_size);
        a.mapped = fixed_size;
        MemoryProtect(a.mem, fixed_size);
        a.committed = fixed_size;
    }
    else {
        a.mem = (u8*) MemoryReserve(ARENA_RESERVE_SIZE);
        a.mapped = ARENA_RESERVE_SIZE;
        MemoryProtect(a.mem, ARENA_COMMIT_CHUNK);
        a.committed = ARENA_COMMIT_CHUNK;
    }

    return a;
}
void ArenaDestroy(MArena *a) {
    MemoryUnmap(a, a->committed);
    *a = {};
}
inline
void *ArenaAlloc(MArena *a, u64 len, bool zerod = true) {
    if (a->fixed_size) {
        assert(a->fixed_size == a->committed && "ArenaAlloc: fixed_size misconfigured");
        assert(a->fixed_size <= a->used + len && "ArenaAlloc: fixed_size exceeded");
    }

    else if (a->committed < a->used + len) {
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
inline
void ArenaRelease(MArena *a, u64 len) {
    assert(len <= a->used);

    a->used -= len;
}
inline
void *ArenaPush(MArena *a, void *data, u32 len) {
    void *dest = ArenaAlloc(a, len);
    _memcpy(dest, data, len);
    return dest;
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


struct MPoolBlockHdr {
    MPoolBlockHdr *next;
    u64 lock;
};

struct MPool {
    u8 *mem;
    u32 block_size;
    u32 nblocks;
    u32 occupancy;
    u64 lock;
    MPoolBlockHdr free_list;
};


#define MPOOL_MIN_BLOCK_SIZE 64
static MPool pool_zero;


MPool PoolCreate(u32 block_size_min, u32 nblocks) {
    assert(nblocks > 1);

    MPool p = pool_zero;;
    p.block_size = MPOOL_MIN_BLOCK_SIZE * (block_size_min / MPOOL_MIN_BLOCK_SIZE + 1);
    p.nblocks = nblocks;
    p.lock = (u64) &p; // this number is a lifetime constant

    p.mem = (u8*) MemoryReserve(p.block_size * p.nblocks);
    MemoryProtect(p.mem, p.block_size * p.nblocks);

    MPoolBlockHdr *freeblck = &p.free_list;
    for (u32 i = 0; i < nblocks; ++i) {
        freeblck->next = (MPoolBlockHdr*) (p.mem + i * p.block_size);
        freeblck->lock = p.lock;
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

bool PoolFree(MPool *p, void *element, bool enable_strict_mode = true) {
    assert(PoolCheckAddress(p, element) && "input address aligned and in range");
    MPoolBlockHdr *e = (MPoolBlockHdr*) element;

    bool address_aligned = PoolCheckAddress(p, e);
    bool next_address_matches = PoolCheckAddress(p, e->next);
    bool key_matches = (e->lock == p->lock);

    if (key_matches && (next_address_matches || e->next == NULL)) {
        if (enable_strict_mode) {
            assert(1 == 0 && "Attempt to free an un-allocated block");
        }
        else {
            return false;
        }
    }

    else if (address_aligned == false) {
        if (enable_strict_mode) {
            assert(1 == 0 && "Attempt to free a non-pool address");
        }
        else {
            return false;
        }
    }

    else {
        e->next = p->free_list.next;
        e->lock = p->lock;

        p->free_list.next = e;
        --p->occupancy;
    }
    return true;
}

inline
u32 PoolPtr2Idx(MPool *p, void *ptr) {
    PoolCheckAddress(p, ptr);
    if (ptr == NULL) {
        return 0;
    }
    u32 idx = (u32) ( ((u8*) ptr - (u8*) p->mem) / p->block_size );
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

u32 PoolAllocIdx(MPool *p) {
    void *element = PoolAlloc(p);
    if (element == NULL) {
        return 0;
    }

    u32 idx = (u32) ((u8*) element - (u8*) p->mem) / p->block_size;
    assert(idx < p->nblocks && "block index must be positive and less and the number of blocks");

    return idx;
}

bool PoolFreeIdx(MPool *p, u32 idx) {
    void * ptr = PoolIdx2Ptr(p, idx);
    return PoolFree(p, ptr);
}


//
// Templated memory pool wrapper
//


template<typename T>
struct MPoolT {
    MPool _p;

    T *Alloc() {
        return (T*) PoolAlloc(&this->_p);
    }
    void Free(T* el) {
        PoolFree(&this->_p, el);
    }
};
template<class T>
MPoolT<T> PoolCreate(u32 nblocks) {
    MPool pool_inner = PoolCreate(sizeof(T), nblocks);
    MPoolT<T> pool;
    pool._p = pool_inner;
    return pool;
}


//
//  Memory arena context - just a quick way to get some memory up


struct MContext {
    // temporary, persistent, lifetime
    MArena _a_tmp;
    MArena _a_pers;
    MArena _a_life;
    MArena *a_tmp;
    MArena *a_pers;
    MArena *a_life;
};

static MContext _g_mctx;
static MContext *g_mctx;
MContext *GetContext(u64 arenas_fixed_size = 0) {
    if (g_mctx == NULL) {
        g_mctx = &_g_mctx;
        g_mctx->_a_tmp = ArenaCreate(arenas_fixed_size);
        g_mctx->a_tmp = &g_mctx->_a_tmp;
        g_mctx->_a_pers = ArenaCreate(arenas_fixed_size);
        g_mctx->a_pers = &g_mctx->_a_pers;
        g_mctx->_a_life = ArenaCreate(arenas_fixed_size);
        g_mctx->a_life = &g_mctx->_a_life;
    }
    return g_mctx;
}

MArena *GetArenaTemp() {
    MContext *ctx = GetContext();
    return ctx->a_tmp;
}

MArena *GetArenaPers() {
    MContext *ctx = GetContext();
    return ctx->a_pers;
}

MArena *GetArenaLife() {
    MContext *ctx = GetContext();
    return ctx->a_life;
}


//
//  Array


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
    T *AddUnique(T element) {
        for (u32 i = 0; i < len; ++i) {
            if (lst[i] == element) {
                return NULL;
            }
        }
        return Add(element);
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
        // unordered delete / swap-dec
        T swap = Last();
        len--;
        lst[len] = lst[idx];
        lst[idx] = swap;
    }
};
template<class T>
List<T> InitList(MArena *a, u32 count, bool zerod = true) {
    List<T> _lst = {};
    _lst.len = 0;
    _lst.lst = (T*) ArenaAlloc(a, sizeof(T) * count, zerod);
    return _lst;
}
template<class T>
void ArenaShedTail(MArena *a, List<T> lst, u32 diff_T) {
    assert(a->used >= diff_T * sizeof(T));
    assert(a->mem + a->used == (u8*) (lst.lst + lst.len + diff_T));

    a->mem -= diff_T * sizeof(T);
}
template<class T>
List<T> ListCopy(MArena *a_dest, List<T> src) {
    List<T> dest = InitList<T>(a_dest, src.len);
    dest.len = src.len;
    _memcpy(dest.lst, src.lst, sizeof(T) * src.len);
    return dest;
}


//
//  Fixed-size arrays with overflow checks


// TODO: override [] to be able to do the overflow check while using that operator


template<typename T>
struct Array {
    T *arr = NULL;
    u32 len = 0;
    u32 max = 0;

    inline
    void Add(T *element) {
        assert(len < max);

        arr[len++] = *element;
    }
    inline
    T *Add(T element) {
        assert(len < max);

        arr[len++] = element;
        return LastPtr();
    }
    inline
    T *AddUnique(T element) {
        assert(len < max);

        for (u32 i = 0; i < len; ++i) {
            if (arr[i] == element) {
                return NULL;
            }
        }
        return Add(element);
    }
    inline
    void Push(T element) {
        assert(len < max);

        arr[len++] = element;
    }
    inline
    T Pop() {
        if (len) {
            return arr[--len];
        }
        else {
            return {};
        }
    }
    inline
    T Last() {
        if (len) {
            return arr[len - 1];
        }
        else {
            return {};
        }
    }
    inline
    T *LastPtr() {
        if (len) {
            return arr + len - 1;
        }
        else {
            return NULL;
        }
    }
    inline
    T First() {
        if (len) {
            return arr[0];
        }
        else {
            return {};
        }
    }
    inline
    void Delete(u32 idx) {
        assert(idx < len);

        T swap = Last();
        len--;
        arr[len] = arr[idx];
        arr[idx] = swap;
    }
};

template<class T>
Array<T> InitArray(MArena *a, u32 max_len) {
    Array<T> _arr = {};
    _arr.len = 0;
    _arr.max = max_len;
    _arr.arr = (T*) ArenaAlloc(a, sizeof(T) * max_len);
    return _arr;
}


//
//  Stack


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
            T zero = {};
            return zero;
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
// Self-expanding array


// eXpanding list:
//
// - pass by reference, constructor is there
// - memory managed by internal arena
// - array subscripting, accessed by Lst(), can be used up to Len(), done by accessing a pointer member
/*
    ListX<u32> lst;
*/


template<typename T>
struct ListX {
    MArena _arena;

    inline
    void _XPand() {
        ArenaAlloc(&this->_arena, ARENA_COMMIT_CHUNK);
    }
    void Init(u32 initial_cap = 0) {
        this->_arena = ArenaCreate();
        if (Cap() == 0) {
            _XPand();
        }
        while (Cap() < initial_cap) {
            _XPand();
        }
    }
    inline
    T *Lst() {
        return (T*) _arena.mem;
    }
    inline
    u32 Len() {
        assert(this->_arena.used % sizeof(T) == 0);
        u32 len = (u32) this->_arena.used / sizeof(T);
        return len;
    }
    void SetLen(u32 len) {
        Init(len);
        _arena.used = len * sizeof(T);
    }
    inline
    u32 Cap() {
        u32 capacity = (u32) this->_arena.committed / sizeof(T);
        return capacity;
    }
    inline
    void Add(T element) {
        if (sizeof(T) > _arena.committed - _arena.used) {
            _XPand();
        }
        Lst()[Len()] = element;
        this->_arena.used += sizeof(T);
    }
    inline
    T *GetPtr(u32 idx) {
        return (T*) _arena.mem + idx;
    }
    inline
    T Get(u32 idx) {
        return *((T*) _arena.mem + idx);
    }
    inline
    void Set(u32 idx, T element) {
        if (idx < Len()) {
            Lst()[idx] = element;
        }
    }
};


//
// Stretchy buffer
//
// Subscripting on the native C pointer. (E.g. using ptr[])
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


//
//  Sort
//


void SortBubbleU32(List<u32> arr) {
    u32 a;
    u32 b;
    for (u32 i = 0; i < arr.len; ++i) {
        for (u32 j = 0; j < arr.len - 1; ++j) {
            a = arr.lst[j];
            b = arr.lst[j+1];

            if (a > b) {
                arr.lst[j] = b;
                arr.lst[j+1] = a;
            }
        }
    }
}


void SortQuickU32() {
    // TODO: impl.
}


List<u32> SetIntersectionU32(MArena *a_dest, List<u32> arr_a, List<u32> arr_b) {
    List<u32> result = InitList<u32>(a_dest, 0);
    if (arr_a.len == 0 || arr_b.len == 0) {
        return result;
    }
    SortBubbleU32(arr_a);
    SortBubbleU32(arr_b);

    u32 min = MaxU32(arr_a.First(), arr_b.First());
    u32 max = MinU32(arr_a.Last(), arr_b.Last());
    u32 i = 0;
    u32 j = 0;
    u32 i_max = arr_a.len - 1;
    u32 j_max = arr_b.len - 1;

    // fast-forward both arrays to min value
    // TODO: fast-forward using divide-and-conquer
    u32 a = arr_a.First();
    u32 b = arr_b.First();
    while (a < min) {
        a = arr_a.lst[++i];
    }
    while (b < min) {
        b = arr_b.lst[++j];
    }

    // do the intersection work until we reach max value
    while (i <= i_max && j <= j_max && a <= max && b <= max) {
        if (a == b) {
            if (result.len == 0 || (result.len > 0 && a != result.Last())) {
                ArenaAlloc(a_dest, sizeof(u32));
                result.Add(a);
            }
            a = arr_a.lst[++i];
            b = arr_b.lst[++j];
        }
        else if (a < b) {
            while (a < b && i < i_max) {
                a = arr_a.lst[++i];
            }
        }
        else if (b < a) {
            while (b < a && j < j_max) {
                b = arr_b.lst[++j];
            }
        }
    }
    return result;
}


//
//  string.h
//


//
// Str and StrLst
//


struct Str {
    char *str = NULL;
    u32 len = 0;
};


struct StrLst {
    char *str;
    u32 len;
    StrLst *next;
    StrLst *first;

    Str GetStr() {
        return Str { str, len };
    }
    void SetStr(char * s) {
        str = s;
        len = _strlen(s);
    }
};


char *StrZeroTerm(MArena *a, Str s) {
    char * result = (char*) ArenaAlloc(a, s.len + 1);
    _memcpy(result, s.str, s.len);
    result[s.len] = 0;
    return result;
}
Str StrLiteral(MArena *a, const char *lit) {
    Str s;
    s.len = 0;
    while (*(lit + s.len) != '\0') {
        ++s.len;
    }
    s.str = (char*) ArenaAlloc(a, s.len);
    _memcpy(s.str, lit, s.len);

    return s;
}
static char str_prnt_buff[512];
void StrPrint(const char *format, Str s) {
    // TODO: get variadic
    assert(1 == 0 && "not implemented");

    //sprintf(str_prnt_buff, "%.*s", s.len, s.str);
    //printf(format, str_prnt_buff);
}
inline void StrPrint(Str s) {
    printf("%.*s", s.len, s.str);
}
inline void StrPrint(const char *aff, Str s, const char *suf) {
    printf("%s%.*s%s", aff, s.len, s.str, suf);
}
inline void StrPrint(StrLst s) {
    printf("%.*s", s.len, s.str);
}
inline void StrPrint(Str *s) {
    printf("%.*s", s->len, s->str);
}
inline void StrPrint(StrLst *s) {
    printf("%.*s", s->len, s->str);
}
bool StrEqual(Str a, Str b) {
    u32 i = 0;
    u32 len = MinU32(a.len, b.len);
    while (i < len) {
        if (a.str[i] != b.str[i]) {
            return false;
        }
        ++i;
    }

    return a.len == b.len;
}
bool StrContainsChar(Str s, char c) {
    for (u32 i = 0; i < s.len; ++i) {
        if (c == s.str[i]) {
            return true;
        }
    }
    return false;
}
Str StrCat(MArena *arena, Str a, Str b) {
    Str cat;
    cat.len = a.len + b.len;
    cat.str = (char*) ArenaAlloc(arena, cat.len);
    _memcpy(cat.str, a.str, a.len);
    _memcpy(cat.str + a.len, b.str, b.len);

    return cat;
}
void StrLstSetToFirst(StrLst **lst) {
    if ((*lst)->first) {
        *lst = (*lst)->first;
    }
}
u32 StrListLen(StrLst *lst, u32 limit = -1) {
    if (lst == NULL) {
        return 0;
    }
    StrLstSetToFirst(&lst);

    u32 cnt = 0;
    while (lst && cnt < limit) {
        cnt++;
        lst = lst->next;
    }
    return cnt;
}
void StrLstPrint(StrLst *lst, const char *sep = "\n") {
    while (lst != NULL) {
        StrPrint(lst); // TODO: fix
        printf("%s", sep);
        lst = lst->next;
    }
}
StrLst *_StrLstAllocNext(MArena *a_dest) {
    static StrLst def;
    StrLst *lst = (StrLst*) ArenaPush(a_dest, &def, sizeof(StrLst));
    lst->str = (char*) ArenaAlloc(a_dest, 0);
    return lst;
}
StrLst *StrSplit(MArena *a_dest, Str base, char split) {
    StrLst *next = _StrLstAllocNext(a_dest);
    StrLst *first = next;
    StrLst *node = next;

    u32 i = 0;
    u32 j = 0;
    while (i < base.len) {
        // seek
        j = 0;
        while (i + j < base.len && base.str[i + j] != split) {
            j++;
        }

        // copy
        if (j > 0) {
            if (node->len > 0) {
                next = _StrLstAllocNext(a_dest);
                node->next = next;
                node->first = first;
                node = next;
            }

            node->len = j;
            ArenaAlloc(a_dest, j);
            _memcpy(node->str, base.str + i, j);
        }

        // iter
        i += j + 1;
    }
    return first;
}
StrLst *StrSplitSpacesKeepQuoted(MArena *a_dest, Str base) {
    char space = ' ';
    char quote = '"';

    u32 qcnt = 0;
    bool e1 = false;
    bool e2 = false;
    for (u32 i = 0; i < base.len; ++i) {
        if (base.str[i] == quote) {
            qcnt++;

            // check whether the quotation thing is padded with space on left/right side (uneven / even)
            bool qts_are_wrapped_by_spaces =
                (qcnt % 2 == 1) && (i == 0 || base.str[i - 1] == ' ') ||
                (qcnt % 2 == 0) && (i == base.len - 1 || base.str[i + 1] == ' ');

            if (!qts_are_wrapped_by_spaces) {
                e1 = true;
            }
        }
    }
    e2 = qcnt % 2 == 1;

    bool debug_print = false;
    if (e1 || e2) {
        if (debug_print) printf("FAIL: qcnt: %u\n", qcnt);
        if (debug_print) printf("FAIL: fail %d %d\n", e1, e2);
        return NULL;
    }
    else {
        if (debug_print) printf("able to collapse\n");
    }

    char split = space;
    StrLst *next = _StrLstAllocNext(a_dest);
    StrLst *first = next;
    StrLst *node = next;

    u32 i = 0;
    u32 j = 0;
    while (i < base.len) {
        // seek
        j = 0;
        while (i + j < base.len && (base.str[i + j] != split) ) {
            char c = base.str[i + j];

            if (c == quote) {
                i++;
                if (split == space) {
                    split = quote;
                }
                else {
                    split = space;
                }
            }

            j++;
        }

        // copy
        if (j > 0) {
            if (node->len > 0) {
                next = _StrLstAllocNext(a_dest);
                node->next = next;
                node->first = first;
                node = next;
            }

            node->len = j;
            ArenaAlloc(a_dest, j);
            _memcpy(node->str, base.str + i, j);
        }

        // iter
        split = space;
        i += j + 1;
    }
    return first;
}
Str StrJoin(MArena *a, StrLst *strs) {
    // TODO: we should just ArenaAlloc() every time there is a new string

    u32 amount_needed = 0;
    while (strs != NULL) {
        amount_needed += strs->len;
        strs = strs->next;
    }

    Str join;
    join.str = (char*) ArenaAlloc(a, amount_needed);
    join.len = 0;
    while (strs != NULL) {
        _memcpy(join.str + join.len, strs->str, strs->len);
        join.len += strs->len;
        strs = strs->next;
    }

    return join;
}
Str StrJoinInsertChar(MArena *a, StrLst *strs, char insert) {
    // TODO: we should just ArenaAlloc() every time there is a new string

    u32 amount_needed = 0;
    while (strs != NULL) {
        amount_needed += strs->len;

        strs = strs->next;
        if (strs != NULL) {
            amount_needed++;
        }
    }

    Str join;
    join.str = (char*) ArenaAlloc(a, amount_needed);
    join.len = 0;
    while (strs != NULL) {
        _memcpy(join.str + join.len, strs->str, strs->len);
        join.len += strs->len;
        strs = strs->next;

        if (strs != NULL) {
            join.str[join.len] = insert;
            ++join.len;
        }
    }

    return join;
}
Str StrTrim(MArena *a, Str s, char t) {
    if (s.len) {
        if (s.str[0] == t) {
            s.str++;
            s.len -=1;
        }
        if (s.str[s.len-1] == t) {
            s.len -=1;
        }
    }
    return s;
}


//
// string list builder functions [another take]


StrLst *StrLstPush(MArena *a, char *str, StrLst *after = NULL) {
    StrLst _ = {};
    StrLst *lst = (StrLst*) ArenaPush(a, &_, sizeof(StrLst));
    lst->len = _strlen(str);
    lst->str = (char*) ArenaAlloc(a, lst->len + 1);
    lst->str[lst->len] = 0;

    for (u32 i = 0; i < lst->len; ++i) {
        lst->str[i] = str[i];
    }
    if (after != NULL) {
        assert(after->first && "enforce first is set");

        after->next = lst;
        lst->first = after->first;
    }
    else {
        lst->first = lst;
    }
    return lst;
}
StrLst *StrLstPush(MArena *a, StrLst *lst, char *str) {
    
    // USAGE: e.g.
    //
    //  StrLst lst = NULL;
    //  lst = StrLstPush(lst, "hello strs");

    return StrLstPush(a, str, lst);
}
char *StrLstNext(MArena *a, StrLst **lst) {
    char *str = (*lst)->str;
    *lst = (*lst)->next;
    return str;
}


void StrLstPrint(StrLst lst) {
    StrLst *iter = &lst;
    do {
        StrPrint(iter);
        printf("\n");
    }
    while ((iter = iter->next) != NULL);
}
StrLst *StrLstPop(StrLst *pop, StrLst *prev) {
    if (pop == NULL) {
        return NULL;
    }
    else if (pop->first == NULL && pop->next == NULL) {
        return NULL;
    }

    // pop is first
    else if (pop == pop->first) { 
        assert(prev == NULL);

        StrLst *newfirst = pop->next;
        StrLst *iter = newfirst;
        while (iter) {
            iter->first = newfirst;
            iter = iter->next;
        }
        pop->first = newfirst; // this line is just a safeguard in case someone uses first on the item
        return newfirst;
    }

    else if (prev != NULL) {
        assert(prev->next == pop);

        // pop is in the middle
        if (pop->next) {
            prev->next = pop->next;
        }

        // pop is the last
        else {
            prev->next = NULL;
        }
    }

    return pop->first;
}


//
//  Automated arena signatures


static MArena _g_a_strings;
static MArena *g_a_strings;
MArena *StringCreateArena() {
    if (g_a_strings == NULL) {
        _g_a_strings = ArenaCreate();
        g_a_strings = &_g_a_strings;
    }
    return g_a_strings;
}
MArena *StringInit() {
    return StringCreateArena();
}
void StringSetGlobalArena(MArena *a) {
    g_a_strings = a;
}
MArena *StringGetGlobalArena() {
    return g_a_strings;
}
MArena *InitStrings() {
    return StringCreateArena();
}


//
//  Wrappers without any arena arg, these just expand on the currently set arena


inline
char *StrZeroTerm(Str s) {
    return StrZeroTerm(g_a_strings, s);
}
inline
Str StrLiteral(const char *literal) {
    return StrLiteral(g_a_strings, literal);
}
inline
Str StrInline(const char *literal) {
    return Str { (char*) literal, _strlen((char*) literal) };
}
inline
Str StrLiteral(char *literal) {
    return StrLiteral(g_a_strings, (const char*) literal);
}
#define StrL StrLiteral
inline
bool StrEqual(Str a, const char *lit) {
    Str b = StrLiteral(lit);
    return StrEqual(a, b);
}
inline
Str StrAlloc(MArena *a_dest, u32 len) {
    char *buff = (char*) ArenaAlloc(a_dest, len);
    return Str { buff, len };
}
inline
Str StrAlloc(u32 len) {
    char *buff = (char*) ArenaAlloc(g_a_strings, len);
    return Str { buff, len };
}
inline
Str StrCat(Str a, Str b) {
    return StrCat(g_a_strings, a, b);
}
inline
Str StrCat(Str a, char *b) {
    return StrCat(g_a_strings, a, StrLiteral(b));
}
inline
Str StrCat(Str a, const char *b) {
    return StrCat(g_a_strings, a, StrLiteral(b));
}
inline
Str StrCat(const char *a, Str b) {
    return StrCat(g_a_strings, StrLiteral(a), b);
}
inline
StrLst *StrSplit(Str base, char split) {
    return StrSplit(g_a_strings, base, split);
}
inline
StrLst *StrSplitSpacesKeepQuoted(Str base) {
    return StrSplitSpacesKeepQuoted(g_a_strings, base);
}
inline
StrLst *StrSplitLines(Str base) {
    return StrSplit(g_a_strings, base, '\n');
}
inline
StrLst *StrSplitWords(Str base) {
    return StrSplit(g_a_strings, base, ' ');
}
inline
Str StrJoin(StrLst *strs) {
    return StrJoin(g_a_strings, strs);
}
inline
Str StrTrim(Str s, char t) {
    return StrTrim(g_a_strings, s, t);
}
inline
Str StrJoinInsertChar(StrLst *strs, char insert) {
    return StrJoinInsertChar(g_a_strings, strs, insert);
}
inline
StrLst *StrLstPush(char *str, StrLst *after = NULL) {
    return StrLstPush(g_a_strings, str, after);
}
inline
StrLst *StrLstPush(Str str, StrLst *after = NULL) {
    return StrLstPush(g_a_strings, StrZeroTerm(str), after);
}
inline
char *StrLstNext(StrLst **lst) {
    return StrLstNext(g_a_strings, lst);
}
StrLst *StrLstPush(StrLst *lst, char *str) {
    return StrLstPush(g_a_strings, lst, str);
}


//
//  hash.h
//


//
// hash map


u32 Hash32(u32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
u64 Hash64(u64 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
#ifdef __arm__
    #warning "__arm__ detected: u64 typedef'd to 32bit"
    #define Hash Hash32
#else
    #define Hash Hash64
#endif

u64 HashStringValue(Str skey) {
    u32 key4 = 0;
    for (u32 i = 0; i < skey.len; ++i) {
        u32 val = skey.str[i] << i % 4;
        key4 += val;
    }

    u64 hashval = Hash(key4);
    return hashval;
}

u32 HashStringValue(Str key, u32 mod) {
    u32 key4 = 0;
    for (u32 i = 0; i < key.len; ++i) {
        u32 val = key.str[i] << i % 4;
        key4 += val;
    }
    u32 slot = Hash(key4) % mod;
    return slot;
}

u64 HashStringValue(const char *key) {
    u32 key4 = 0;
    u32 i = 0;
    while ( key[i] != '\0' ) {
        u32 val = key[i] << i % 4;
        key4 += val;

        ++i;
    }
    u64 hashval = Hash(key4);
    return hashval;
}


//
//  Pointer-map
//
//  (You need to supply a 64b key)
//


// TODO: enable a key-iteration option


struct HashMapKeyVal {
    u64 key;
    u64 val;
    HashMapKeyVal *chain;
};
struct HashMap {
    List<HashMapKeyVal> slots;
    List<HashMapKeyVal> colls;
    u32 ncollisions;
    u32 nresets;
    u32 noccupants;

    bool IsInitialized() {
        return slots.len > 0;
    }
    void PrintOccupance() {
        printf("noccupants: %u, nresets: %u\n", noccupants, nresets);
    }
};
HashMap InitMap(MArena *a_dest, u32 nslots = 1000) {
    HashMap map = {};
    map.slots = InitList<HashMapKeyVal>(a_dest, nslots);
    map.slots.len = nslots;
    map.colls = InitList<HashMapKeyVal>(a_dest, nslots);

    return map;
}
void MapClear(HashMap *map) {
    u32 nslots = map->slots.len;
    _memzero(map->colls.lst, sizeof(HashMapKeyVal) * map->slots.len);
    _memzero(map->slots.lst, sizeof(HashMapKeyVal) * map->slots.len);
    map->ncollisions = 0;
    map->noccupants = 0;
    map->nresets = 0;
    map->colls.len = 0;
    map->slots.len = nslots;
}

struct MapIter {
    s32 slot_idx;
    s32 coll_idx;

    s32 occ_slots_cnt = 0;
    s32 occ_colliders_cnt = 0;
};

u64 MapNextVal(HashMap *map, MapIter *iter) {
    while (iter->slot_idx < map->slots.len) {
        HashMapKeyVal kv = map->slots.lst[iter->slot_idx++];

        if (kv.val) {
            iter->occ_slots_cnt++;

            return kv.val;
        }

    }
    while (iter->coll_idx < map->colls.len) {
        HashMapKeyVal kv = map->colls.lst[iter->coll_idx++];
        if (kv.val) {
            iter->occ_colliders_cnt++;

            return kv.val;
        }
    }

    return 0;
}

bool MapPut(HashMap *map, u64 key, u64 val) {
    assert(key != 0 && "null ptr can not be used as a key");

    u32 slot = Hash(key) % map->slots.len;
    HashMapKeyVal *kv_slot = map->slots.lst + slot;

    if (kv_slot->key == 0 || kv_slot->key == key) {
        if (kv_slot->key == key) {
            map->noccupants--;
            map->nresets++;
        }
        // new slot or reset value
        HashMapKeyVal kv;
        kv.key = key;
        kv.val = val;
        kv.chain = kv_slot->chain;
        map->slots.lst[slot] = kv;
    }
    else {
        // collision
        map->ncollisions++;

        HashMapKeyVal *collider = kv_slot;
        while (collider->chain != NULL) {
            collider = collider->chain;
        }
        if (map->colls.len == map->slots.len) {
            // no more space
            return false;
        }

        // put a new collider onto the list
        HashMapKeyVal kv_new = {};
        kv_new.key = key;
        kv_new.val = val;

        collider->chain = map->colls.Add(kv_new);
    }
    map->noccupants++;
    return true;
}
inline
bool MapPut(HashMap *map, void *key, void *val) {
    return MapPut(map, (u64) key, (u64) val);
}

inline
bool MapPut(HashMap *map, u64 key, void *val) {
    return MapPut(map, key, (u64) val);
}

inline
bool MapPut(HashMap *map, Str skey, void *val) {
    return MapPut(map, HashStringValue(skey), (u64) val);
}

u64 MapGet(HashMap *map, u64 key) {
    u32 slot = Hash(key) % map->slots.len;
    HashMapKeyVal kv_slot = map->slots.lst[slot];

    if (kv_slot.key == key) {
        return kv_slot.val;
    }
    else {
        HashMapKeyVal *kv = &kv_slot;
        while (kv->chain != NULL) {
            kv = kv->chain;
            if (kv->key == key) {
                return kv->val;
            }
        }
    }

    return 0;
}
inline
u64 MapGet(HashMap *map, Str skey) {
    return MapGet(map, HashStringValue(skey));
}

bool MapRemove(HashMap *map, u64 key, void *val) {
    u32 slot = Hash(key) % map->slots.len;
    HashMapKeyVal kv_slot = map->slots.lst[slot];

    if (kv_slot.key == key) {
        map->slots.lst[slot] = {};
        return true;
    }
    else {
        HashMapKeyVal *kv_prev = map->slots.lst + slot;
        HashMapKeyVal *kv = kv_slot.chain;
        while (kv != NULL) {
            if (kv->key == key) {
                kv_prev->chain = kv->chain;
                *kv = {};
                return true;
            }
            else {
                kv_prev = kv;
                kv = kv->chain;
            }
        }
    }

    return false;
}


//
// random


// TODO: reference ReadCPUTimer()


#ifndef ULONG_MAX
#  define ULONG_MAX ( (u64) 0xffffffffffffffffUL )
#endif

void Kiss_SRandom(u64 state[7], u64 seed) {
    if (seed == 0) seed = 1;
    state[0] = seed | 1; // x
    state[1] = seed | 2; // y
    state[2] = seed | 4; // z
    state[3] = seed | 8; // w
    state[4] = 0;        // carry
}
u64 Kiss_Random(u64 state[7]) {
    state[0] = state[0] * 69069 + 1;
    state[1] ^= state[1] << 13;
    state[1] ^= state[1] >> 17;
    state[1] ^= state[1] << 5;
    state[5] = (state[2] >> 2) + (state[3] >> 3) + (state[4] >> 2);
    state[6] = state[3] + state[3] + state[2] + state[4];
    state[2] = state[3];
    state[3] = state[6];
    state[4] = state[5] >> 30;
    return state[0] + state[1] + state[3];
}
u64 g_state[7];
#define McRandom() Kiss_Random(g_state)
u32 RandInit(u32 seed = 0) {
    if (seed == 0) {
        seed = (u32) Hash(ReadSystemTimerMySec());
    }
    Kiss_SRandom(g_state, seed);
    Kiss_Random(g_state); // flush the first one

    return seed;
}

f64 Rand01() {
    f64 randnum = (f64) McRandom();
    randnum /= (f64) ULONG_MAX + 1;
    return randnum;
}
f32 Rand01_f32() {
    f32 randnum = (f32) McRandom();
    randnum /= (f32) ULONG_MAX + 1;
    return randnum;
}
f32 RandPM1_f32() {
    f32 randnum = (f32) McRandom();
    randnum /= ((f32) ULONG_MAX + 1) / 2;
    randnum -= 1;
    return randnum;
}
int RandMinMaxI(int min, int max) {
    assert(max > min);
    return McRandom() % (max - min + 1) + min;
}
u32 RandMinMaxU(u32 min, u32 max) {
    assert(max > min);
    return McRandom() % (max - min + 1) + min;
}
u32 RandMinU16(u32 min) {
    return RandMinMaxU(min, (u16) -1);
}
f32 RandMinMaxI_f32(int min, int max) {
    assert(max > min);
    return (f32) (McRandom() % (max - min + 1) + min);
}
int RandDice(u32 max) {
    assert(max > 0);
    return McRandom() % max + 1;
}
int RandIntMax(u32 max) {
    assert(max > 0);
    return McRandom() % max + 1;
}

void PrintHex(u8* data, u32 len) {
    const char *nibble_to_hex = "0123456789ABCDEF";

    if (data) {
        for (u32 i = 0; i < len; ++i) {
            u8 byte = data[i];
            char a = nibble_to_hex[byte >> 4];
            char b = nibble_to_hex[byte & 0x0F];
            printf("%c%c ", a, b);

            if (i % 4 == 3 || (i == len + 1)) {
                printf("\n");
            }
        }
    }
}
void WriteRandomHexStr(char* dest, int nhexchars, bool put_newline_and_nullchar = true) {
    RandInit();

    // TODO: make use of the cool "nibble_to_hex" technique (see PrintHex)

    for (int i = 0; i < nhexchars ; i++) {
        switch (RandMinMaxI(0, 15)) {
            case 0: { *dest = '0'; break; }
            case 1: { *dest = '1'; break; }
            case 2: { *dest = '2'; break; }
            case 3: { *dest = '3'; break; }
            case 4: { *dest = '4'; break; }
            case 5: { *dest = '5'; break; }
            case 6: { *dest = '6'; break; }
            case 7: { *dest = '7'; break; }
            case 8: { *dest = '8'; break; }
            case 9: { *dest = '9'; break; }
            case 10: { *dest = 'a'; break; }
            case 11: { *dest = 'b'; break; }
            case 12: { *dest = 'c'; break; }
            case 13: { *dest = 'd'; break; }
            case 14: { *dest = 'e'; break; }
            case 15: { *dest = 'f'; break; }
            default: { assert(1==0); break; }
        };
        dest++;
    }

    if (put_newline_and_nullchar) {
        *dest = '\n';
        dest++;
        *dest = '\0';
    }
}


//
//  utils.h
//


#include <time.h>


//
// Other


void XSleep(u32 ms);


//
// file I/O


u8 *LoadFileMMAP(char *filepath, u64 *size_bytes);
u8 *LoadFileMMAP(const char *filepath, u64 *size_bytes) {
    return LoadFileMMAP((char*) filepath, size_bytes);
}
StrLst *GetFilesInFolderPaths(MArena *a, char *rootpath);
StrLst *GetFilesInFolderPaths(MArena *a, const char *rootpath) {
    return GetFilesInFolderPaths(a, (char*) rootpath);
}

u32 LoadFileGetSize(char* filepath) {
    FILE * f = fopen(filepath, "r");
    if (f == NULL) {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    u32 len = ftell(f);
    return len;
}

u32 LoadFileGetSize(const char* filepath) {
    return LoadFileGetSize((char*) filepath);
}

u32 LoadFileFSeek(char* filepath, void* dest) {
    assert(dest != NULL && "data destination must be valid");
    u32 len = 0;

    FILE * f = fopen(filepath, "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        fseek(f, 0, SEEK_SET);
        fread(dest, 1, len, f);
        fclose(f);
    }
    else {
        printf("LoadFileFSeek: Could not open file: %s\n", filepath);
    }

    return len;
}

void *LoadFileFSeek(MArena *a_dest, char *filepath, u32 *size = NULL) {
    u32 sz = LoadFileGetSize(filepath);
    if (sz == 0) {
        return NULL;
    }

    void *dest = ArenaAlloc(a_dest, sz);
    LoadFileFSeek(filepath, dest);

    if (size != NULL) {
        *size = sz;
    }
    return dest;
}

void *LoadFileFSeek(MArena *a_dest, const char *filepath, u32 *size = NULL) {
    return LoadFileFSeek(a_dest, (char*) filepath, size);
}

bool SaveFile(char *filepath, u8 *data, u32 len);
bool SaveFile(const char *filepath, u8 *data, u32 len) {
    // const char star
    return SaveFile((char *)filepath, data, len);
}
bool SaveFile(char *filepath, void *data, u32 len) {
    // void star
    return SaveFile((char *)filepath, (u8*)data, len);
}
bool SaveFile(const char *filepath, void *data, u32 len) {
    // const char star and void star
    return SaveFile((char *)filepath, (u8*)data, len);
}

bool ArenaSave(MArena *a, char *filename) {
    return SaveFile(filename, a->mem, (u32) a->used);
}
bool ArenaSave(MArena *a, const char *filename) {
    return ArenaSave(a, (char *) filename);
}


//
// path / filename stuff


Str StrBasename(char *path) {
    assert(g_a_strings != NULL && "init strings first");

    // TODO: shouldn't this fail?

    Str before_ext = StrSplit(StrLiteral(path), '.')->GetStr();
    StrLst* slashes = StrSplit(before_ext, '/');
    while (slashes->next) {
        slashes = slashes->next;
    }
    return slashes->GetStr();
}
Str StrBasename(MArena *a, char *path) {
    assert(g_a_strings != NULL && "init strings first");

    // TODO: shouldn't this fail?

    Str before_ext = StrSplit(a, StrLiteral(a, path), '.')->GetStr();
    StrLst* slashes = StrSplit(a, before_ext, '/');
    while (slashes->next) {
        slashes = slashes->next;
    }
    return slashes->GetStr();
}
Str StrBasename(Str path) {
    return StrBasename(StrZeroTerm(path));
}
Str StrExtension(MArena *a, char *path) {
    Str s { NULL, 0 };
    StrLst *lst = StrSplit(a, StrLiteral(a, path), '.');
    if (lst->next != NULL) {
        s = lst->next->GetStr();
    }
    return s;
}
Str StrExtension(char *path) {
    assert(g_a_strings != NULL && "init strings first");

    Str s { NULL, 0 };
    StrLst *lst = StrSplit(StrLiteral(path), '.');
    while (lst->next != NULL) {
        lst = lst->next;
    }
    s = lst->GetStr();
    return s;
}
Str StrExtension(Str path) {
    return StrExtension(StrZeroTerm(path));
}
Str StrDirPath(Str path) {
    assert(g_a_strings != NULL && "init strings first");

    StrLst *slash = StrSplit(path, '/');
    u32 len = StrListLen(slash);

    Str cat = StrL("");
    for (u32 i = 0; i < len - 1; ++i) {
        cat = StrCat(cat, slash->GetStr());
        cat = StrCat(cat, "/");
        slash = slash->next;
    }
    return cat;
}
Str StrPathBuild(Str dirname, Str basename, Str ext) {
    dirname = StrTrim(dirname, '/');
    basename = StrTrim(basename, '/');

    Str path = StrCat(dirname, "/");
    path = StrCat(path, basename);
    path = StrCat(path, ".");
    path = StrCat(path, ext);
    return path;
}
Str StrPathJoin(Str path_1, Str path_2) {
    Str path = StrCat(path_1, "/");
    path = StrCat(path, path_2);
    return path;
}
StrLst *GetFilesExt(const char *extension, const char *path = ".") {
    StrLst *all = GetFilesInFolderPaths(InitStrings(), path);
    StrLst *filtered = NULL;
    StrLst *first = NULL;
    Str ext = StrLiteral(extension);
    while (all != NULL) {
        Str _fpath = all->GetStr();
        Str _ext = StrExtension(_fpath);

        if (StrEqual(_ext, ext)) {
            filtered = StrLstPush(_fpath, filtered);
            if (first == NULL) {
                first = filtered;
            }
        }
        all = all->next;
    }
    return first;
}


struct FInfo {
    Str name;
    Str ext;
    Str basename;
    Str path;
    Str dirname;

    Str BuildName(const char *prefix, const char *suffix, const char *ext) {
        Str bn_new = StrL(prefix);
        bn_new = StrCat(bn_new, basename);
        bn_new = StrCat(bn_new, suffix);
        Str rebuilt = StrPathBuild(dirname, bn_new, StrL(ext));
        return rebuilt;
    }
    char *BuildNameZ(const char *prefix, const char *suffix, const char *ext) {
        return StrZeroTerm( this->BuildName(prefix, suffix, ext) );
    }
    Str StripDirname() {
        Str filename = StrL("");
        filename = StrCat(filename, basename);
        filename = StrCat(filename, ".");
        filename = StrCat(filename, ext);
        return filename;
    }
    void Print() {
        StrPrint("name      : ", name, "\n");
        StrPrint("extension : ", ext, "\n");
        StrPrint("basename  : ", basename, "\n");
        StrPrint("dirname   : ", dirname, "\n");
        Str rebuilt = StrPathBuild(dirname, basename, ext);
        StrPrint("rebuilt   : ", rebuilt, "\n");
    }
};
FInfo FInfoGet(Str pathname) {
    FInfo info;
    info.name = pathname;
    info.ext = StrExtension(pathname);
    info.basename = StrBasename(pathname);
    info.dirname = StrDirPath(pathname);
    return info;
}
FInfo InitFInfo(Str pathname) {
    return FInfoGet(pathname);
}
inline
FInfo FInfoGet(const char*pathname) {
    return FInfoGet(StrL(pathname));
}


Str GetYYMMDD() {
    Str s;

    time_t t = time(NULL);
    struct tm loc = *localtime(&t);
    char buff[34];
    sprintf(buff, "%02d%02d%02d",
        loc.tm_year - 100,
        loc.tm_mon + 1,
        loc.tm_mday);
    printf("%s\n", buff);

    return s;
}


//
// Baselayer initialization


MContext *InitBaselayer() {
    RandInit();
    StringInit();
    return GetContext();
}

void BaselayerAssertVersion(u32 major, u32 minor, u32 patch) {
    if (
        BASELAYER_VERSION_MAJOR != major ||
        BASELAYER_VERSION_MINOR != minor ||
        BASELAYER_VERSION_PATCH != patch
    ) {
        assert(1 == 0 && "baselayer version check failed");
    }
}

void BaselayerPrintVersion() {
    printf("%d.%d.%d\n", BASELAYER_VERSION_MAJOR, BASELAYER_VERSION_MINOR, BASELAYER_VERSION_PATCH);
}



//
//  platform.h
//


// platform-specific implementations of functions
// not currently distributed in separate files


const char *getBuild() { // courtesy of S.O.
    #if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
    #elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    return "x86_32";
    #elif defined(__ARM_ARCH_2__)
    return "ARM2";
    #elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
    return "ARM3";
    #elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
    return "ARM4T";
    #elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
    return "ARM5"
    #elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
    return "ARM6T2";
    #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
    return "ARM6";
    #elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    return "ARM7";
    #elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    return "ARM7A";
    #elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    return "ARM7R";
    #elif defined(__ARM_ARCH_7M__)
    return "ARM7M";
    #elif defined(__ARM_ARCH_7S__)
    return "ARM7S";
    #elif defined(__aarch64__) || defined(_M_ARM64)
    return "ARM64";
    #elif defined(mips) || defined(__mips__) || defined(__mips)
    return "MIPS";
    #elif defined(__sh__)
    return "SUPERH";
    #elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
    return "POWERPC";
    #elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
    return "POWERPC64";
    #elif defined(__sparc__) || defined(__sparc)
    return "SPARC";
    #elif defined(__m68k__)
    return "M68K";
    #else
    return "UNKNOWN";
    #endif
}


#if defined __linux__ || defined __linux
    #define LINUX 1
    #define WINDOWS 0

    #ifdef __arm__
        #define RPI 1
    #endif
    #ifdef __aarch64__
        #define RPI 1
    #endif

    #ifndef RPI
    #define RPI 0
    #endif

    //
    //  plaf_lnx.cpp
    //

        #include <sys/mman.h>
        #include <sys/stat.h>
        #include <sys/time.h>
        #include <sys/mman.h>
        #include <dirent.h>
        #include <unistd.h>


        // TODO: experiment with <x86intrin.h> alongside <sys/time.h> for the straight up __rdtsc() call
        // TODO: impl. LoadFile


        //
        // memory.c


        u64 MemoryProtect(void *from, u64 amount) {
            mprotect(from, amount, PROT_READ | PROT_WRITE);
            return amount;
        }
        void *MemoryReserve(u64 amount) {
            void *result;

            result = (u8*) mmap(NULL, amount, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
            return result;
        }
        s32 MemoryUnmap(void *at, u64 amount_reserved) {
            s32 ret = munmap(at, amount_reserved);
            return ret;
        }



        //
        // profile.c


        u64 ReadSystemTimerMySec() {
            u64 systime;
            struct timeval tm;
            gettimeofday(&tm, NULL);
            systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

            return systime;
        }
        u32 ReadSystemTimerMySec32() {
            u32 systime;
            struct timeval tm;
            gettimeofday(&tm, NULL);
            systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

            return systime;
        }
        u64 ReadCPUTimer() {
            u64 ticks = 0;
            #ifndef __arm__
            #ifndef __aarch64__
            ticks = __builtin_ia32_rdtsc(); // gcc
            #endif
            #endif
            // TODO: test __rdtsc(); with x86intrin.h ! (might increase compile time by a lot)
            return ticks;
        }


        //
        // utils.c


        void XSleep(u32 ms) {
            usleep(1000 * ms);
        }

        u8 *LoadFileMMAP(char *filepath, u64 *size_bytes) {
            FILE *f = fopen(filepath, "rb");
            if (f == NULL) {
                printf("Could not open file: %s\n", filepath);
                return NULL;
            }

            s32 fd = fileno(f);
            struct stat sb;
            if (fstat(fd, &sb) == -1) {
                printf("Could not get file size: %s\n", filepath);
                return NULL;
            }

            u8 *data = (u8*) mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE | MAP_SHARED, fd, 0);
            if (size_bytes != NULL) {
                *size_bytes = sb.st_size;
            }
            fclose(f);
            return data;
        }


        StrLst *GetFilesInFolderPaths(MArena *unused, char *rootpath) {
            StrLst *first = NULL;

            struct dirent *dir;
            DIR *d = opendir(rootpath);
            if (d) {
                d = opendir(rootpath);

                Str path = StrLiteral(rootpath);
                if (path.len == 1 && path.str[0] == '.') {
                    path.len = 0;
                }
                else if (path.str[path.len-1] != '/') {
                    path = StrCat(path, "/");
                }

                StrLst *lst = NULL;
                while ((dir = readdir(d)) != NULL) {
                    if (!_strcmp(dir->d_name, ".") || !_strcmp(dir->d_name, "..")) {
                        continue;
                    }

                    Str dname = StrCat( path, StrLiteral(dir->d_name) );
                    lst = StrLstPush(dname, lst);
                    if (first == NULL) {
                        first = lst;
                    }
                    //Str StrCat( lst->GetStr(), StrLiteral(dir->d_name), );
                }
                closedir(d);
            }
            return first;
        }

        StrLst *GetFilePaths_Rec(char *rootpath, StrLst *head = NULL, StrLst *tail = NULL, const char *extension_filter = NULL, bool do_recurse = true) {
            struct dirent *dir_entry;

            if (DIR *dir = opendir(rootpath)) {
                dir = opendir(rootpath);

                Str path = StrLiteral(rootpath);
                if (path.len == 1 && path.str[0] == '.') {
                    path.len = 0;
                }
                else if (path.str[path.len-1] != '/') {
                    path = StrCat(path, "/");
                }

                while ((dir_entry = readdir(dir)) != NULL) {
                    if (!_strcmp(dir_entry->d_name, ".") || !_strcmp(dir_entry->d_name, "..")) {
                        continue;
                    }

                    Str file_path = StrCat( path, StrLiteral(dir_entry->d_name) );
                    if (head == NULL) {
                        head = tail;
                    }

                    if (dir_entry->d_type == 4) { // recurse into directory
                        if (do_recurse) {
                            tail = GetFilePaths_Rec( StrZeroTerm(file_path), head, tail, extension_filter, true);
                        }
                    }
                    else {
                        if (extension_filter == NULL || StrEqual(StrExtension(StrL(dir_entry->d_name)), extension_filter)) {
                            tail = StrLstPush(file_path, tail);
                            if (false) { StrPrint("", file_path, "\n"); }
                        }
                    }
                }
                closedir(dir);
            }

            else if (FILE *f = fopen(rootpath, "r")) {
                tail = StrLstPush(rootpath, tail);
                fclose(f);
            }

            if (tail == NULL) {
                tail = StrLstPush( Str {}, NULL );
            }

            return tail;
        }

        StrLst *GetFiles(char *rootpath, const char *extension_filter = NULL, bool do_recurse = true) {
            StrLst *fpaths = GetFilePaths_Rec(rootpath, NULL, NULL, extension_filter, do_recurse)->first;
            return fpaths;
        }

        bool SaveFile(char *filepath, u8 *data, u32 len) {
            FILE *f = fopen(filepath, "w");

            if (f == NULL) {
                printf("SaveFile: Could not open file %s\n", filepath);
                return false;
            }

            fwrite(data, 1, len, f);
            fclose(f);

            return true;
        }

#else 
    #define LINUX 0
    #define WINDOWS 1
    #define RPI 0


    //
    //  plaf_lnx.cpp
    //


        #define WIN32_LEAN_AND_MEAN
        #define NOMINMAX
        #include <windows.h>
        #include <fstream>
        #include <fileapi.h>
        #include <intrin.h>


        // TODO: win impl. GetFilesInFolderPaths, LoadFile, LoadFileMMAP


        //
        // memory.h


        u64 MemoryProtect(void *from, u64 amount) {
            VirtualAlloc(from, amount, MEM_COMMIT, PAGE_READWRITE);
            return amount;
        }
        void *MemoryReserve(u64 amount) {
            void *result;
            result = VirtualAlloc(NULL, amount, MEM_RESERVE, PAGE_NOACCESS);
            return result;
        }
        s32 MemoryUnmap(void *at, u64 amount_reserved) {
            bool ans = VirtualFree(at, 0, MEM_RELEASE);
            if (ans == true) {
                return 0;
            }
            else {
                return 1;
            }
        }


        //
        // profile.h


        u64 ReadSystemTimerMySec() {
            // systime (via S.O. 10905892)

            static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
            SYSTEMTIME  system_time;
            FILETIME    file_time;
            uint64_t    time;
            GetSystemTime( &system_time );
            SystemTimeToFileTime( &system_time, &file_time );
            time =  ((uint64_t)file_time.dwLowDateTime )      ;
            time += ((uint64_t)file_time.dwHighDateTime) << 32;

            long tv_sec  = (long) ((time - EPOCH) / 10000000L);
            long tv_usec = (long) (system_time.wMilliseconds * 1000);
            u64 systime = (u32) tv_sec*1000000 + tv_usec; // microsecs 
            return systime;
        }
        u32 ReadSystemTimerMySec32() {
            // systime (via S.O. 10905892)

            static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
            SYSTEMTIME  system_time;
            FILETIME    file_time;
            uint64_t    time;
            GetSystemTime( &system_time );
            SystemTimeToFileTime( &system_time, &file_time );
            time =  ((uint64_t)file_time.dwLowDateTime )      ;
            time += ((uint64_t)file_time.dwHighDateTime) << 32;

            long tv_sec  = (long) ((time - EPOCH) / 10000000L);
            long tv_usec = (long) (system_time.wMilliseconds * 1000);
            u32 systime = (u32) tv_sec*1000000 + tv_usec; // microsecs 

            return systime;
        }
        u64 ReadCPUTimer() {
            u64 rd = __rdtsc();
            return rd;
        }


        //
        // utils.h


        void XSleep(u32 ms) {
            Sleep(ms);
        }


        u8 *LoadFileMMAP(char *filepath, u64 *size_bytes) {
            HANDLE f, map;
            LARGE_INTEGER fsz;

            f = CreateFile(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if (f == INVALID_HANDLE_VALUE) {
                printf("Could not open file: %s\n", filepath);
                return NULL;
            }

            if (!GetFileSizeEx(f, &fsz)) {
                fprintf(stderr, "GetFileSize failed with error %d\n", GetLastError());
                CloseHandle(f);
                return NULL;
            }
            if (size_bytes != NULL) {
                *size_bytes = fsz.QuadPart;
            }

            map = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
            u8 *data = (u8*) MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
            if (data == NULL) {
                printf("Could not load file: %s\n", filepath);
                return NULL;
            }

            return data;
        }
        StrLst *GetFilesInFolderPaths(MArena *a, char *rootpath) {
            StrLst *first = NULL;


            WIN32_FIND_DATA fd_file;
            HANDLE h_find = FindFirstFile(rootpath, &fd_file);
            if (h_find == INVALID_HANDLE_VALUE) {
                printf("invalid: %s\n", rootpath);
            }

            char with_windows_star[200];
            sprintf(with_windows_star, "%s/*", rootpath);
            h_find = FindFirstFile(with_windows_star, &fd_file);
            if (h_find != NULL) {
                StrLst *lst = NULL;

                Str path = StrLiteral(rootpath);
                if (path.len == 1 && path.str[0] == '.') {
                    path.len = 0;
                }
                else if (path.str[path.len-1] != '/') {
                    path = StrCat(path, "/");
                }

                while (FindNextFile(h_find, &fd_file)) {
                    // omit "." and ".."
                    if (!_strcmp(fd_file.cFileName, ".") || !_strcmp(fd_file.cFileName, "..")) {
                        continue;
                    }

                    // next strlst node
                    lst = StrLstPut(a, rootpath, lst);

                    Str dname = StrCat( path, StrLiteral(fd_file.cFileName) );
                    lst = StrLstPut(dname, lst);
                    if (first == NULL) {
                        first = lst;
                    }
                }
                FindClose(h_find);
            }

            return first;
        }
        bool SaveFile(char *filepath, u8 *data, u32 len) {
            std::ofstream outstream(filepath, std::ios::out | std::ios::binary);
            outstream.write((const char*) data, len);
            bool result = outstream.good();
            outstream.close();

            return result;
        }


#endif


#if LINUX
#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif
#ifndef DLL_IMPORT
#define DLL_IMPORT
#endif
#elif WINDOWS
#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif
#ifndef DLL_IMPORT
#define DLL_IMPORT __declspec(dllimport)
#endif
#endif


#ifndef DLL_CLIENT
#ifndef DLL_IMPORTEXPORT
#define DLL_IMPORTEXPORT DLL_EXPORT
#endif
#else
#ifndef DLL_IMPORTEXPORT
#define DLL_IMPORTEXPORT DLL_IMPORT
#endif
#endif



#endif
