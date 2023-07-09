#include <cstdlib>
#include <cstdio>

#include <cstdint>
#include <cassert>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "base.c"
#include "profile.c"
#include "memory.c"
#include "utils.c"
#include "string.c"

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
    String s1 = StrLiteral(&arena, "hello");
    String s2 = StrLiteral(&arena, "hello_noteq");
    String s3 = StrLiteral(&arena, "hello");

    printf("StrPrint - ");
    StrPrint("%s", s1);
    printf("\n");

    printf("StrEqual - ");
    printf("should be (0 1 0): %d %d %d\n", StrEqual(s1, s2), StrEqual(s1, s3), StrEqual(s2, s3));

    printf("StrCat - ");
    StrPrint("%s", StrCat(&arena, s1, s3));
    printf("\n");

    printf("StrSplit / StrLstPrint - ");
    String to_split = StrLiteral(a, "...Hello.I..Have.Been.Split..");
    printf("splitting: ");
    StrPrint("%s", to_split);
    printf(" into: ");
    StringList *lst = StrSplit(a, to_split, '.');
    StrLstPrint(lst);
    printf("\n");

    printf("StrJoin - ");
    String join = StrJoin(a, lst);
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

    TimePrint;
}
