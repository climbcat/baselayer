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

    MArena _b = ArenaCreate();
    MArena *b = &_b;
    u32 width = 1280;
    u32 height = 800;
    GameLoopOne *loop = InitGameLoopOne(b, width, height);

    // data memory pool
    MArena _pointcloud_arena = ArenaCreate();
    MArena *a = &_pointcloud_arena;

    //
    // just set up entities, the rest is automated

    SwRenderer *r = loop->GetRenderer();
    EntitySystem _entity_system = InitEntitySystem();
    EntitySystem *es = &_entity_system;

    Entity *axes = EntityCoordAxes(es, r);
    Entity *box = EntityAABox(es, { 0.3f, 0.0f, 0.7f }, 0.2f, r);
    Entity *box2 = EntityAABox(es, { 0.3f, 0.0f, -0.7f }, 0.2f, r);
    Entity *box3 = EntityAABox(es, { -0.7f, 0.0f, 0.0f }, 0.2f, r);

    box->tpe = ET_LINES_ROT;
    box2->tpe = ET_LINES_ROT;
    box3->tpe = ET_LINES_ROT;

    EntitySystemPrint(es);


    //
    //


    // frame loop
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
