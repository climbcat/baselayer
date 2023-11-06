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


    //
    // just set up entities, the rest is automated


    // data memory pool
    MArena _a2 = ArenaCreate();
    MArena *a2 = &_a2;

    Entity axes = InitAndActivateCoordAxes(loop.GetRenderer());
    Entity box = InitAndActivateAABox({ 0.3, 0, 0.7 }, 0.2, loop.GetRenderer());
    Entity box2 = InitAndActivateAABox({ 0.3, 0.0, -0.7 }, 0.2, loop.GetRenderer());
    Entity box3 = InitAndActivateAABox({ -0.7, 0, 0.0 }, 0.2, loop.GetRenderer());

    box.tpe = ET_LINES_ROT;
    box2.tpe = ET_LINES_ROT;
    box3.tpe = ET_LINES_ROT;

    EntitySystem es;
    EntitySystemChain(&es, &axes);
    EntitySystemChain(&es, &box);
    EntitySystemChain(&es, &box2);
    EntitySystemChain(&es, &box3);

    PointCloud pc_1;
    {
        RandInit();
        u32 npoints = 90;
        List<Vector3f> points = InitList<Vector3f>(a2, npoints);
        Vector3f min { -2, -2, -2 };
        Vector3f max { 2, 2, 2 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            *(points.lst + points.len++) = Vector3f {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
        }
        pc_1 = InitPointCloud(points);
        pc_1._entity.color = { RGBA_BLUE };
    }
    PointCloud pc_2;
    {
        RandInit();
        u32 npoints = 300;
        List<Vector3f> points = InitList<Vector3f>(a2, npoints);
        Vector3f min { -2, -2, -2 };
        Vector3f max { 2, 2, 0 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            *(points.lst + points.len++) = Vector3f {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
        }
        pc_2 = InitPointCloud(points);
        pc_2._entity.color = { RGBA_GREEN };
    }
    PointCloud pc_3;
    {
        RandInit();
        u32 npoints = 600;
        List<Vector3f> points = InitList<Vector3f>(a2, npoints);
        Vector3f min { -2, -2, -2 };
        Vector3f max { 0, 0, 0 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            *(points.lst + points.len++) = Vector3f {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
        }
        pc_3 = InitPointCloud(points);
        pc_3._entity.color = { RGBA_RED };
    }

    EntitySystemChain(&es, &pc_1._entity);
    EntitySystemChain(&es, &pc_2._entity);
    EntitySystemChain(&es, &pc_3._entity);
    EntitySystemPrint(&es);


    //
    //


    // frame loop
    while (loop.GameLoopRunning()) {
        // E.g.: update entity transforms
        // E.g.: update debug UI
        // E.g.: run simulations
        // E.g.: pull worker thread statuses

        loop.CycleFrame(&es);
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
