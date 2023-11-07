#include <GL/glew.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <signal.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../baselayer.h"
#include "geometry.h"
#include "shaders.h"
#include "ui.h"
#include "swrender.h"
#include "test.cpp"


void RunProgram() {
    printf("Soft rendering demo ...\n");

    u32 width = 1280;
    u32 height = 800;
    GameLoopOne loop = InitGameLoopOne(width, height);

    // data memory pool
    MArena _pointcloud_arena = ArenaCreate();
    MArena *a = &_pointcloud_arena;


    //
    // just set up entities, the rest is automated


    EntitySystem _entity_system = InitEntitySystem();
    EntitySystem *es = &_entity_system;

    Entity *axes = InitAndActivateCoordAxes(es, loop.GetRenderer());
    Entity *box = InitAndActivateAABox(es, { 0.3, 0, 0.7 }, 0.2, loop.GetRenderer());
    Entity *box2 = InitAndActivateAABox(es, { 0.3, 0.0, -0.7 }, 0.2, loop.GetRenderer());
    Entity *box3 = InitAndActivateAABox(es, { -0.7, 0, 0.0 }, 0.2, loop.GetRenderer());

    box->tpe = ET_LINES_ROT;
    box2->tpe = ET_LINES_ROT;
    box3->tpe = ET_LINES_ROT;

    Entity *pc_1;
    {
        RandInit();
        u32 npoints = 90;

        EntityStream *hdr = InitEntityStream(a, DT_VERTICES, npoints);
        pc_1 = InitAndActivateDataEntity(es, hdr, loop.GetRenderer());
        pc_1->color = { RGBA_BLUE };

        List<Vector3f> points { (Vector3f*) hdr->GetData(), npoints };
        Vector3f min { -2, -2, -2 };
        Vector3f max { 2, 2, 2 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f v {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
            points.lst[i] = v;
            points.len++;
        }
    }

    Entity *pc_2;
    {
        RandInit();
        u32 npoints = 300;

        EntityStream *hdr = InitEntityStream(a, DT_VERTICES, npoints, pc_1->entity_stream);
        pc_2 = InitAndActivateDataEntity(es, hdr, loop.GetRenderer());
        pc_2->color = { RGBA_GREEN };

        List<Vector3f> points { (Vector3f*) hdr->GetData(), npoints };
        Vector3f min { -2, -2, -2 };
        Vector3f max { 2, 2, 0 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f v {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
            points.lst[i] = v;
            points.len++;
        }
    }
    Entity *pc_3;
    {
        RandInit();
        u32 npoints = 600;

        EntityStream *hdr = InitEntityStream(a, DT_VERTICES, npoints, pc_2->entity_stream);
        pc_3 = InitAndActivateDataEntity(es, hdr, loop.GetRenderer());
        pc_3->color = { RGBA_RED };

        List<Vector3f> points { (Vector3f*) hdr->GetData(), npoints };
        Vector3f min { -2, -2, -2 };
        Vector3f max { 0, 0, 0 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f v {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
            points.lst[i] = v;
            points.len++;
        }
    }
    EntitySystemPrint(es);
    EntityStreamPrint(pc_1->entity_stream, (char*) "random point clouds");


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
}


int main (int argc, char **argv) {
    TimeProgram;
    bool force_testing = false;

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
