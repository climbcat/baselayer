#include "baselayer.h"
#include "graphics/geometry.h"
#include "algorithms/octree.h"
#include "algorithms/indices.h"

#include "test.cpp"


void RunProgram() {
    printf("Usage:\n");
    printf("<tool>\n");
    printf("<tool> --help\n");
    printf("<tool> --test\n");
}


int main (int argc, char **argv) {
    TimeProgram;
    bool forcetest = true;

    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage: <tool> <args>\n");
        printf("--help:         Display help (this text)\n");
        printf("--test:         Run test functions\n");
    }
    else if (CLAContainsArg("--test", argc, argv) || forcetest) {
        Test();
    }
    else {
        RunProgram();
    }
}
