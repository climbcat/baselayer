#include <cstdlib>
#include <cstdio>

#include <cstdint>
#include <cassert>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "base.c"
#include "memory.c"
#include "utils.c"
#include "string.c"
#include "profile.c"

void RunProgram() {
    printf("Running program...\n");
}

void RunTests() {
    printf("Running tests...\n");
}

int main (int argc, char **argv) {
    if (CLAContainsArg("--help", argc, argv) || argc != 2) {
        printf("Usage:\n        <example>\n");
        exit(0);
    }
    if (CLAContainsArg("--test", argc, argv)) {
        RunTests();
        exit(0);
    }
    RunProgram();

    //TimePrint;
}
