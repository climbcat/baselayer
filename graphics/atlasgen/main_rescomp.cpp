
#include "../../baselayer.h"
#include "../gtypes.h"
#include "../atlas.h"

#include "../resource.h"

#include "cmunrm.cpp"


void RunProgram() {
    TimeFunction;
    MContext *ctx = InitBaselayer();
    LoadResource(ctx->a_life, cmunrm);
}


int main (int argc, char **argv) {
    TimeProgram;

    if (CLAContainsArg("--n1", argc, argv)) {
        u32 n1 = ParseInt( CLAGetArgValue("--n1", argc, argv) );
    }

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
    }
    else {
        RunProgram();
    }
}
