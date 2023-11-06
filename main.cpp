#include "baselayer.h"
#include "graphics/geometry.h"


struct PoolTestEntity {
    Matrix4f a;
    Matrix4f b;
    Matrix4f c;
};


void RunProgram() {
    printf("Just a baselayer entry point\n");
}


void RunTests() {
    TimeFunction;
    printf("Running tests ...\n");

    MArena arena = ArenaCreate();
    MArena *a = &arena;

    printf("StrLiteral\n");
    Str s1 = StrLiteral(&arena, "hello");
    Str s2 = StrLiteral(&arena, "hello_noteq");
    Str s3 = StrLiteral(&arena, "hello");

    printf("StrPrint - ");
    StrPrint("%s", s1);
    printf("\n");

    printf("StrEqual - ");
    printf("should be (0 1 0): %d %d %d\n", StrEqual(s1, s2), StrEqual(s1, s3), StrEqual(s2, s3));

    printf("StrCat - ");
    StrPrint("%s", StrCat(&arena, s1, s3));
    printf("\n");

    printf("StrSplit / StrLstPrint - ");
    Str to_split = StrLiteral(a, "...Hello.I..Have.Been.Split..");
    printf("splitting: ");
    StrPrint("%s", to_split);
    printf(" into: ");
    StrLst *lst = StrSplit(a, to_split, '.');
    StrLstPrint(lst);
    printf("\n");

    printf("StrJoin - ");
    Str join = StrJoin(a, lst);
    StrPrint("%s", join);
    printf("\n");
    join = StrJoinInsertChar(a, lst, '/');
    StrPrint("%s", join);
    printf("\n");

    printf("CLAInit - ");
    if (CLAContainsArg("--test", g_argc, g_argv)) {
        printf("OK");
    }
    printf("\n");

    // StrLst & get files in folder
    printf("files in folder '.': \n");
    StrLst files = GetFilesInFolderPaths(a, (char*) ".");
    StrLstPrint(files);

    // templated list
    ListX<u32> lst_T;
    lst_T.Add(14);
    lst_T.Add(222);
    lst_T.At(1);

    // stretchy buffer
    printf("\nstretchy buffer:\n");
    s32 *elst = NULL;
    for (int i = 0; i < 10000; ++i) {
        lst_push(elst, i);
        if (i > 10000 - 5) {
            printf("%d\n", elst[i]);
        }
    }
    lst_free(elst);

    // random numbers
    RandInit();
    for (int i = 0; i < 10; ++i)  {
        f64 r = Rand01();
        printf("Rand01: %f\n", r);
    }
    printf("RandDice: %u\n\n", RandDice(20));

    // save binary data
    u32 num_chars = 1024*1024 + 1;
    char data[num_chars];
    WriteRandomHexStr(data, num_chars, true);
    char *filepath = (char*) "hexdata.txt";
    SaveFile(filepath, (u8*) data, num_chars);
    printf("Saved binary hex chars to file hexdata.txt\n\n");

    // load using C fseek
    u8* dest = (u8*) malloc(num_chars);
    u32 nbytesloaded = LoadFileFSeek(filepath, dest);
    assert(num_chars == nbytesloaded);
    printf("Loaded %d bytes back in using fseek\n\n", nbytesloaded);

    // memory mapped load
    u64 num_chars_64 = (u64) num_chars;
    u8 *data_mmapped = LoadFileMMAP(filepath, &num_chars_64);
    printf("Memory mapped load %ld nbytes:\n", num_chars_64);
    printf("%.1000s\n\n", (char*) data_mmapped);

    // testing the memory pool
    printf("testing mem pool:\n");
    u32 pool_size = 1001;
    u32 test_num_partial = 666;
    u32 test_num_rand = 14;
    MPool pool = PoolCreate(sizeof(PoolTestEntity), pool_size);
    PoolTestEntity *e;
    PoolTestEntity *elements[pool_size];
    // populate to full
    printf("populating to max ...\n");
    for (u32 i = 0; i < pool_size; ++i) {
        e = (PoolTestEntity *) PoolAlloc(&pool);
        elements[i] = e;
        assert(e != NULL);
    }
    e = (PoolTestEntity *) PoolAlloc(&pool);
    assert(e == NULL);
    assert(pool.occupancy == pool_size);
    // depopulate to zero
    printf("de-populating to zero ...\n");
    for (u32 i = 0; i < pool_size; ++i) {
        e = elements[i];
        PoolFree(&pool, e);
    }
    assert(pool.occupancy == 0);
    // re-populate to 50%
    printf("re-populating to partial ...\n");
    for (u32 i = 0; i < test_num_partial; ++i) {
        e = (PoolTestEntity *) PoolAlloc(&pool);
        elements[i] = e;
        assert(e != NULL);
    }
    assert(pool.occupancy == test_num_partial);
    // blip randomly

    // TODO: fix the bug, pool isn't working 100% correctly

    printf("blipping randomly ... ");
    RandInit();
    u32 hits = 0;
    u32 tries = 0;
    while (hits < test_num_rand) {
        u32 slot = RandDice(pool_size) - 1;
        printf(" %d", slot);
        e = elements[slot];


        if (slot >= test_num_partial) {
            printf("oops");
        }

        if (PoolFree(&pool, e, false)) {
            ++hits;
            if (slot >= test_num_partial) {
                printf("oops");
            }
            assert(slot < test_num_partial);
        }
        else {
            assert(slot >= test_num_partial);
        }
        ++tries;
    }
    printf("\nblips: %d tries: %d\n", hits, tries);
    // TODO: populate to full occupancy
    //printf("populating back to full occupance ...\n");
}


int main (int argc, char **argv) {
    TimeProgram;
    bool forcetest = true;

    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage:\n        <example>\n");
    }
    else if (CLAContainsArg("--test", argc, argv) || forcetest) {
        RunTests();
    }
    else {
        RunProgram();
    }
}
