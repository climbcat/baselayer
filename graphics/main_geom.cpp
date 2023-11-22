#include "../baselayer.h"
#include "geometry.h"
#include "shaders.h"
#include "ui.h"
#include "swrender.h"
#include "gameloop.h"
#include "test.cpp"


void RunProgram() {
    printf("Soft rendering demo ...\n");

    u32 width = 1280;
    u32 height = 800;
    GameLoopOne loop = InitGameLoopOne(width, height);
    InitGameLoopWindowPointer_Hack(&loop);

    // data memory pool
    MArena _pointcloud_arena = ArenaCreate();
    MArena *a = &_pointcloud_arena;

    //
    // just set up entities, the rest is automated

    SwRenderer *r = loop.GetRenderer();
    EntitySystem _entity_system = InitEntitySystem();
    EntitySystem *es = &_entity_system;

    Entity *axes = InitAndActivateCoordAxes(es, loop.GetRenderer());
    Entity *box = InitAndActivateAABox(es, { 0.3, 0, 0.7 }, 0.2, r);
    Entity *box2 = InitAndActivateAABox(es, { 0.3, 0.0, -0.7 }, 0.2, r);
    Entity *box3 = InitAndActivateAABox(es, { -0.7, 0, 0.0 }, 0.2, r);

    box->tpe = ET_LINES_ROT;
    box2->tpe = ET_LINES_ROT;
    box3->tpe = ET_LINES_ROT;

    EntitySystemPrint(es);


    //
    //


    // frame loop
    while (loop.GameLoopRunning()) {
        // E.g.: update entity transforms
        // E.g.: update debug UI
        // E.g.: run simulations
        // E.g.: pull worker thread statuses

        loop.CycleFrame(es);
    }
    loop.Terminate();
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

    return 0;
}
