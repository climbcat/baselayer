#include "../baselayer.h"
#include "geometry.h"
#include "octree.h"
#include "shaders.h"
#include "ui.h"
#include "stream.h"
#include "entity.h"
#include "swrender.h"
#include "gameloop.h"
#include "test.cpp"


void RunProgram() {
    printf("Soft rendering demo ...\n");

    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();

    Entity *axes = EntityCoordAxes(es, r);
    Entity *box = EntityAABox(es, { 0.3f, 0.0f, 0.7f }, 0.2f, r);
    box->tpe = ET_LINES_ROT;
    Entity *box2 = EntityAABox(es, { 0.3f, 0.0f, -0.7f }, 0.2f, r);
    box2->tpe = ET_LINES_ROT;
    Entity *box3 = EntityAABox(es, { -0.7f, 0.0f, 0.0f }, 0.2f, r);
    box3->tpe = ET_LINES_ROT;
    EntitySystemPrint(es);

    while (loop->GameLoopRunning()) {
        // E.g.: update entity transforms
        // E.g.: update debug UI
        // E.g.: run simulations
        // E.g.: pull worker thread statuses

        loop->CycleFrame(es);
    }
    loop->Terminate();
}


int main (int argc, char **argv) {
    TimeProgram;
    bool force_testing = true;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
    }
    else if (CLAContainsArg("--test", argc, argv) || force_testing) {
        Test();
    }
    else {
        RunProgram();
    }

    return 0;
}
