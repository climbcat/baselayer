#include <cstdio>
#include <cassert>

#include "base.c"
#include "profile.c"
#include "memory.c"
#include "string.c"
#include "utils.c"
#include "platform.c"

void RunProgram() {
    printf("Just a baselayer entry point\n");
}

void RunTests() {
    TimeFunction;

    printf("Running tests ...\n");

    printf("ArenaCreate\n");
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

    // test get files in folder & a bit of str lst 
    StrLst files = GetFilesInFolderPaths(a, (char*) "/home");
    StrLstPrint(files);

    // templated list
    ListX<u32> lst_T;
    lst_T.Add(14);
    lst_T.Add(222);
    lst_T.At(1);

    // stretchy buffer 
    s32 *elst = NULL;
    lst_push(elst, 42);
    lst_push(elst, -15);
    for (int i = 0; i < lst_len(elst); ++i) {
        printf("%d\n", elst[i]);
    }
    lst_free(elst);
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
