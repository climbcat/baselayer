#ifndef __HASH_H__
#define __HASH_H__


//
// hash map


u32 Hash(u32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}


// TODO: also be a GPA?
// TODO: if max-len key (200 chars?), storage can be an array with
//      slot_size = sz(hdr) + 200 + sz(T), and with some tricks we can
//      also do un-ordered delete for removal, compressing it always


struct DictKeyVal {
    DictKeyVal *chain;  // collision chaining
    DictKeyVal *nxt;    // walk to next
    DictKeyVal *prv;    // walk to prev
    Str key;            // the full key
    void *val;          // the value
    u32 sz_val;
};


struct Dict {
    // fixed-size values dict with string keys
    u32 nslots;
    u32 sz_val;
    MArena *a_storage;
    List<u64> slots;
    DictKeyVal *head;
    DictKeyVal *tail;
    u32 ncollisions;
};
Dict InitDict(u32 nslots = 256, u32 sz_val = 0) {
    Dict dct;
    _memzero(&dct, sizeof(Dict));
    dct.nslots = nslots;
    dct.sz_val = sz_val;
    dct.a_storage = ArenaCreateBootstrapped();
    dct.slots = InitList<u64>(dct.a_storage, nslots);
    dct.slots.len = nslots;

    return dct;
}


// TODO: how do we remove chained values?


u64 DictStoragePush(Dict *dct, Str key, void *val, u32 sz_val, u64 slot_ptr) {
    DictKeyVal *hdr = NULL;

    // reset old value
    bool collision = false;
    bool resetval = false;
    if (slot_ptr != 0) {
        assert(dct->head != NULL);
        assert(dct->tail != NULL);

        DictKeyVal *old = (DictKeyVal*) slot_ptr;
        if (StrEqual(key, old->key)) {
            assert(sz_val == old->sz_val && "TODO: reset value size");

            hdr = old;
            resetval = true;
        }
        else {
            collision = true;
            dct->ncollisions++;
        }
    }

    if (hdr == NULL) {
        hdr = (DictKeyVal*) ArenaAlloc(dct->a_storage, sizeof(DictKeyVal));
    }

    if (resetval == true) {
        // TODO: !?
    }
    else if (dct->head == NULL) {
        assert(dct->tail == NULL);
        assert(resetval == false);

        dct->head = hdr;
        dct->tail = hdr;
    }
    else {
        assert(dct->tail != NULL);
        assert(dct->tail->nxt == NULL || resetval);

        dct->tail->nxt = hdr;
        hdr->prv = dct->tail;
        dct->tail = hdr;
    }
    u64 ret_slot_ptr = (u64) hdr;

    // collision
    if (collision) {
        DictKeyVal *collider = (DictKeyVal*) slot_ptr;

        while (collider->chain != NULL) {
            collider = collider->chain;
        }

        if (StrEqual(key, collider->key)) {
            // TODO: its a reset-val
        }

        collider->chain = hdr;

        ret_slot_ptr = slot_ptr;
    }

    hdr->sz_val = sz_val;
    hdr->key.len = key.len;
    hdr->key.str = (char*) ArenaPush(dct->a_storage, key.str, key.len);
    hdr->val = ArenaPush(dct->a_storage, val, sz_val);

    return ret_slot_ptr;
}
void DictStorageWalk(Dict *dct) {
    DictKeyVal *kv = dct->head;
    while (kv != NULL) {
        printf("%s : %u\n", StrZeroTerm(kv->key), *((u32*) kv->val));

        kv = kv->nxt;
    }
}

void DictPut(Dict *dct, Str key, void *val, u32 sz = 0) {
    assert(sz != 0 || dct->sz_val != 0);
    if (sz == 0) {
        sz = dct->sz_val;
    }

    u32 key4 = 0;
    for (u32 i = 0; i < key.len; ++i) {
        u32 val = key.str[i] << i % 4;
        key4 += val;
    }
    u32 slot = Hash(key4) % dct->slots.len;


    // DEBUG observe slot spread somehow
    printf("slot: %u\n", slot);


    u64 ptr = dct->slots.lst[slot];
    ptr = DictStoragePush(dct, key, val, sz, ptr);
    dct->slots.lst[slot] = ptr;
}
void DictPut(Dict *dct, char *key, void *val, u32 sz = 0) {
    return DictPut(dct, Str { key, _strlen(key) }, val, sz);
}
void DictPut(Dict *dct, const char *key, void *val, u32 sz = 0) {
    return DictPut(dct, Str { (char*) key, _strlen( (char*) key) }, val, sz);
}

u32 DictGet(Dict *dct, u32 key) {
    u32 hashed = Hash(key) % dct->slots.len;
    u32 val = dct->slots.lst[hashed];
    return val;
}
/*
u32 DictGet(Dict *dct, Str key) {
    u32 hashed = Hash(key) % dct->slots.len;
    u32 val = dct->slots.lst[hashed];
    return val;
}
*/




//
// random


// TODO: reference ReadCPUTimer()


#ifndef ULONG_MAX
#  define ULONG_MAX ((unsigned long)0xffffffffffffffffUL)
#endif

void Kiss_SRandom(unsigned long state[7], unsigned long seed) {
    if (seed == 0) seed = 1;
    state[0] = seed | 1; // x
    state[1] = seed | 2; // y
    state[2] = seed | 4; // z
    state[3] = seed | 8; // w
    state[4] = 0;        // carry
}
unsigned long Kiss_Random(unsigned long state[7]) {
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
unsigned long g_state[7];
bool g_didinit = false;
#define McRandom() Kiss_Random(g_state)
u32 RandInit(u32 seed = 0) {
    if (g_didinit == true)
        return 0;

    if (seed == 0) {
        seed = Hash(ReadCPUTimer());
    }
    Kiss_SRandom(g_state, seed);
    Kiss_Random(g_state); // flush the first one

    g_didinit = true;
    return seed;
}

f64 Rand01() {
    f64 randnum;
    randnum = (f64) McRandom();
    randnum /= (f64) ULONG_MAX + 1;
    return randnum;
}
f32 Rand01_f32() {
    f32 randnum;
    randnum = (f32) McRandom();
    randnum /= (f32) ULONG_MAX + 1;
    return randnum;
}
f32 Rand0132() {
    f64 num = McRandom();
    num /= (f32) ULONG_MAX + 1;
    return num;
}
f32 RandPM1_f32() {
    f32 randnum;
    randnum = (f32) McRandom();
    randnum /= ((f32) ULONG_MAX + 1) / 2;
    randnum -= 1;
    return randnum;
}
int RandMinMaxI(int min, int max) {
    assert(max > min);
    return McRandom() % (max - min + 1) + min;
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
        for (int i = 0; i < len; ++i) {
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


#endif
