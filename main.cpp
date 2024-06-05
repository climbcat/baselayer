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


void SmallTests() {
    TimeFunction;

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


    //
    // StrLst & get files in folder

    printf("files in folder '.': \n");
    StrLst *files = GetFilesInFolderPaths(a, (char*) ".");
    StrLstPrint(files);


    //
    // templated list

    ListX<u32> lst_T;
    lst_T.Add(14);
    lst_T.Add(222);
    lst_T.Get(1);
    lst_T.GetPtr(1);


    //
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


    //
    // random numbers

    RandInit();
    for (int i = 0; i < 10; ++i)  {
        f64 r = Rand01();
        printf("Rand01: %f\n", r);
    }
    printf("RandDice: %u\n\n", RandDice(20));


    //
    // save binary data

    u32 num_chars = 1024*1024 + 1;
    char data[num_chars];
    WriteRandomHexStr(data, num_chars, true);
    char *filepath = (char*) "hexdata.txt";
    SaveFile(filepath, (u8*) data, num_chars);
    printf("Saved binary hex chars to file hexdata.txt\n\n");


    //
    // load using C fseek

    u8* dest = (u8*) malloc(num_chars);
    u32 nbytesloaded = LoadFileFSeek(filepath, dest);
    assert(num_chars == nbytesloaded);
    printf("Loaded %d bytes back in using fseek\n\n", nbytesloaded);


    //
    // memory mapped load

    u64 num_chars_64 = (u64) num_chars;
    u8 *data_mmapped = LoadFileMMAP(filepath, &num_chars_64);
    printf("Memory mapped load %ld nbytes:\n", num_chars_64);
    printf("%.1000s\n\n", (char*) data_mmapped);


    //
    // memory pool

    printf("testing mem pool:\n");
    u32 pool_size = 1001;
    u32 test_num_partial = 666;
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
        bool twice = PoolFree(&pool, e, false);
        assert(twice == false);
    }
    assert(pool.occupancy == 0);
    // twice-free another element
    e = elements[test_num_partial];
    bool twice2 = PoolFree(&pool, e, false);
    assert(twice2 == false);
    // re-populate to some %
    printf("re-populating to partial ...\n");
    for (u32 i = 0; i < test_num_partial; ++i) {
        e = (PoolTestEntity *) PoolAlloc(&pool);
        elements[i] = e;
        assert(e != NULL);
    }
    assert(pool.occupancy == test_num_partial);
}


void _PrintFlexArrayU32(u32 *arr) {
    for (u32 i = 0; i < lst_len(arr); ++i) {
        printf("%u ", arr[i]);
    }
}
void TestSortingAlgs() {
    printf("TestSortingAlgs\n");

    MContext *ctx = GetContext();
    RandInit();

    u32 cnt_a = 100;
    u32 cnt_b = 300;
    u32 min_a = 11;
    u32 max_a = 44;
    u32 min_b = 22;
    u32 max_b = 66;

    u32 *arr_a = NULL;
    for (u32 i = 0; i < cnt_a; ++i) {
        lst_push(arr_a, RandMinMaxI(min_a, max_a));
    }
    assert(cnt_a == lst_len(arr_a) && "another flexi buffer test");

    u32 *arr_b = NULL;
    for (u32 i = 0; i < cnt_b; ++i) {
        lst_push(arr_b, RandMinMaxI(min_b, max_b));
    }
    assert(cnt_b == lst_len(arr_b) && "another flexi buffer test");

    // print array a
    printf("\narray a:\n");
    _PrintFlexArrayU32(arr_a);
    printf("\n\n");

    // sort array a
    SortBubbleU32({ arr_a, lst_len(arr_a) });
    _PrintFlexArrayU32(arr_a);
    printf("\n\n");

    // print array b
    printf("array b:\n");
    _PrintFlexArrayU32(arr_b);
    printf("\n\n");

    // sort array b
    SortBubbleU32({ arr_b, lst_len(arr_b) });
    _PrintFlexArrayU32(arr_b);
    printf("\n\n");

    //
    printf("intersection:\n");
    List<u32> intersection = SetIntersectionU32(ctx->a_pers, { arr_a, lst_len(arr_a) }, { arr_b, lst_len(arr_b) });
    for (u32 i = 0; i < intersection.len; ++i) {
        printf("%u ", intersection.lst[i]);
    }
    printf("\n\n");
}


void TestStringHelpers() {
    printf("TestStringHelpers\n");

    printf("\nlook for .h hiles in ../graphics:\n");
    StrLst *fs = GetFilesExt("h", "../graphics");
    StrLstPrint(fs);

    printf("\nlook for .cmake file in .:\n");
    StrLst *fs2 = GetFilesExt("cmake", ".");
    StrLstPrint(fs2);
}


void TestDict() {
    printf("TestDict\n");
    InitBaselayer();
    
    u32 nslots = 17;
    u32 sz_val = sizeof(u32);
    Dict dct = InitDict(nslots, sz_val);
    dct.debug_print = true;
    RandInit();

    printf("\n| put values:\n");
    u32 val;
    val = RandIntMax(UINT32_MAX);
    printf("hest : %u in ", val);
    DictPut(&dct, "hest", &val);
    val = RandIntMax(UINT32_MAX);
    printf("melon : %u in ", val);
    DictPut(&dct, "melon", &val);
    val = RandIntMax(UINT32_MAX);
    printf("møg : %u in ", val);
    DictPut(&dct, "møg", &val);
    val = RandIntMax(UINT32_MAX);
    printf("blad : %u in ", val);
    DictPut(&dct, "blad", &val);
    val = RandIntMax(UINT32_MAX);
    printf("appelsin : %u in ", val);
    DictPut(&dct, "appelsin", &val);
    val = RandIntMax(UINT32_MAX);
    printf("æble : %u in ", val);
    DictPut(&dct, "pære", &val);
    val = RandIntMax(UINT32_MAX);
    printf("æble : %u in ", val);
    DictPut(&dct, "æble", &val);
    val = RandIntMax(UINT32_MAX);
    printf("bold : %u in ", val);
    DictPut(&dct, "bold", &val);
    val = RandIntMax(UINT32_MAX);
    printf("sol : %u in ", val);
    DictPut(&dct, "sol", &val);
    val = RandIntMax(UINT32_MAX);
    printf("måne : %u in ", val);
    DictPut(&dct, "måne", &val);
    val = RandIntMax(UINT32_MAX);
    printf("blad : %u in ", val);
    DictPut(&dct, "blad", &val);
    val = RandIntMax(UINT32_MAX);
    printf("gulerod : %u in ", val);
    DictPut(&dct, "gulerod", &val);
    val = RandIntMax(UINT32_MAX);
    printf("banan : %u in ", val);
    DictPut(&dct, "banan", &val);

    printf("\ncollisions: %u\n", dct.ncollisions);
    printf("\n| walk the key-vals:\n");
    DictStorageWalk(&dct);

    // get values 

    printf("\n| getting a few values:\n");
    u32 *ptr_val;
    ptr_val = (u32*) DictGet(&dct, "gulerod");
    printf("gulerod get : %u\n", *ptr_val);
    ptr_val = (u32*) DictGet(&dct, "melon");
    printf("melon get : %u\n", *ptr_val);
    ptr_val = (u32*) DictGet(&dct, "hest");
    printf("hest get : %u\n", *ptr_val);
}


void TestPointerHashMap() {
    printf("TestPointerHashMap\n");
    MContext *ctx = InitBaselayer();

    u32 nslots = 12;
    HashMap _map = InitMap(ctx->a_life, nslots);
    HashMap *map = &_map;

    u32 nputs = 20;
    static u64 keys[20];
    for (u32 i = 0; i < nputs; ++i) {
        // create some random 64b values
        u64 key_high = RandMinMaxU(1, UINT32_MAX - 1);
        u64 key_low = RandMinMaxU(1, UINT32_MAX - 1);
        u64 val_high = RandMinMaxU(1, UINT32_MAX - 1);
        u64 val_low = RandMinMaxU(1, UINT32_MAX - 1);
        u64 key = (key_high << 32) + key_low;
        keys[i] = key;
        u64 val = (val_high << 32) + val_low;
        printf("MapPut() key: %lu, val: %lu\n", key, val);

        // enter into the map
        MapPut(map, key, val);

    }
    printf("collisions: %u, resets: %u\n\n", map->ncollisions, map->nresets);


    for (u32 i = 0; i < nputs; ++i) {
        u64 key = keys[i];
        u64 val = MapGet(map, key);

        printf("MapGet() key: %lu, val: %lu\n", key, val);
    }
}


void TestScritinizeFilename() {
    printf("TestScritinizeFilename\n");
    InitBaselayer();

    Str fname;
    FInfo info;

    printf("\n");
    fname = StrL( "./data/123456_sumfile.ext" );
    info = FInfoGet(fname);
    info.Print();

    printf("\n");
    fname = StrL( "/home/user/data/123456_sumfile.ext" );
    info = FInfoGet(fname);
    info.Print();

    printf("\n");
    fname = StrL( "12345/6789/user/data/123456_sumfile.longfileextention" );
    info = FInfoGet(fname);
    info.Print();

    Str mod = info.BuildName("", "_suffixed", "extaltered");
    printf("\nmod  : ");
    StrPrint(mod);
    printf("\n");

    GetYYMMDD();
}


void Test() {
    printf("Running tests ...\n");

    //SmallTests();
    //TestSortingAlgs();
    //TestStringHelpers();
    //TestDict();
    //TestPointerHashMap();
    TestScritinizeFilename();
}


int main (int argc, char **argv) {
    TimeProgram;
    bool forcetest = true;

    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage:\n        <example>\n");
    }
    else if (CLAContainsArg("--test", argc, argv) || forcetest) {
        Test();
    }
    else {
        RunProgram();
    }
}
