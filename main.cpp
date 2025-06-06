#include "baselayer.h"

#include "test.cpp"


void RunProgram() {
    printf("Usage:\n");
    printf("./baselayer\n");
    printf("./baselayer --help\n");
    printf("./baselayer --test\n");
}


int main (int argc, char **argv) {
    TimeProgram;
    bool force_tests = false;

    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage: ./baselayer <args>\n");
        printf("--help:         Display help (this text)\n");
        printf("--version:      Print baselayer version\n");
        printf("--test:         Run test functions\n");
        exit(0);
    }
    else if (CLAContainsArg("--test", argc, argv) || force_tests) {
        Test();
    }
    else if (CLAContainsArg("--version", argc, argv) || force_tests) {
        BaselayerPrintVersion();
        exit(0);
    }
    else {
        RunProgram();
    }
}
