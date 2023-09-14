#include <cstdio>
#include <cassert>

#include "baselayer.h"

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
    StrLst files = GetFilesInFolderPaths(a, (char*) ".");
    StrLstPrint(files);

    // templated list
    ListX<u32> lst_T;
    lst_T.Add(14);
    lst_T.Add(222);
    lst_T.At(1);

    // stretchy buffer 
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
    printf("Memory mapped %d nbytes:\n", num_chars_64);
    printf("%.1000s\n\n", (char*) data_mmapped);
}


int main (int argc, char **argv) {
    TimeProgram;

    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage:\n        <example>\n");
    }
    else if (CLAContainsArg("--test", argc, argv)) {
        RunTests();
    }
    else {
        RunProgram();
    }
}
