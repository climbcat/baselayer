#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../baselayer.h"
#include "geometry.h"
#include "test.cpp"


void RunProgram() {
    TimeFunction;
    printf("Executing program ...\n");

    // ...
}

int main (int argc, char **argv) {
    TimeProgram;
    bool force_testing = true;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
        exit(0);
    }
    if (CLAContainsArg("--test", argc, argv) || force_testing) {
        Test();
        exit(0);
    }

    RunProgram();
}
