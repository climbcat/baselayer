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

    SwRenderer rend = InitRenderer();
    SwRenderer *r = &rend;
    SDL_Window *window = InitOGL(rend.w, rend.h);

    // image buffer
    ScreenQuadTextureProgram screen;
    screen.Init(rend.image_buffer, rend.w, rend.h);

    // data memory pool
    MArena _a2 = ArenaCreate();
    MArena *a2 = &_a2;

    // entities
    EntitySystem es;
    CoordAxes axes = InitCoordAxes();
    AABox box = InitAABox({ 0.3, 0, 0.7 }, 0.2);
    AABox box2 = InitAABox({ 0.3, 0.0, -0.7 }, 0.2);
    AABox box3 = InitAABox({ -0.7, 0, 0.0 }, 0.2);

    CoordAxesActivate(&axes, r);
    AABoxActivate(&box, r);
    AABoxActivate(&box2, r);
    AABoxActivate(&box3, r);

    EntitySteysmChain(&es, &axes._entity);
    EntitySteysmChain(&es, &box._entity);
    EntitySteysmChain(&es, &box2._entity);
    EntitySteysmChain(&es, &box3._entity);

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

    EntitySteysmChain(&es, &pc_1._entity);
    EntitySteysmChain(&es, &pc_2._entity);
    EntitySteysmChain(&es, &pc_3._entity);
    Entity *next = es.first;

    // print the entity chain: 
    u32 eidx = 0;
    while (next != NULL) {
        printf("%u: vertices %u -> %u lines %u -> %u\n", eidx, next->verts_low, next->verts_high, next->lines_low, next->lines_high);
        eidx++;
        next = next->next;
    }

    // camera
    OrbitCamera cam = InitOrbitCamera(rend.aspect);

    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, false, false, false);
    Matrix4f view, model, mvp;

    u64 iter = 0;
    MouseTrap mouse = InitMouseTrap();
    while (Running(&mouse)) {
        // frame start
        XSleep(10);
        ClearToZeroRGBA(rend.image_buffer, rend.w, rend.h);
        rend.screen_buffer.len = 0;

        // orbit camera
        cam.Update(&mouse, true);

        // entity loop (POC): vertices -> NDC
        u32 eidx = 0;
        next = es.first;

        while (next != NULL) {
            if (next ->tpe != ET_POINTCLOUD) {
                if (next->tpe == ET_AXES) {
                    model = next->transform;
                }
                else {
                    model = next->transform * TransformBuildRotateY(0.03f * iter);
                }
                mvp = TransformBuildMVP(model, cam.view, proj);

                // lines to screen buffer
                for (u32 i = next->verts_low; i <= next->verts_high; ++i) {
                    rend.ndc_buffer.lst[i] = TransformPerspective(mvp, rend.vertex_buffer.lst[i]);
                }
                // render lines
                LinesToScreen(rend.w, rend.h, &rend.screen_buffer, &rend.index_buffer, &rend.ndc_buffer, next->lines_low, next->lines_high, next->color);
            }
            else {
                mvp = TransformBuildMVP(Matrix4f_Identity(), cam.view, proj);

                // render pointcloud
                RenderPointCloud(rend.image_buffer, rend.w, rend.h, &mvp, ((PointCloud*)next)->points, next->color);
            }
            eidx++;
            next = next->next;
        }
        rend.ndc_buffer.len = rend.vertex_buffer.len;
        iter++;
        for (u32 i = 0; i < rend.screen_buffer.len / 2; ++i) {
            RenderLineRGBA(rend.image_buffer, rend.w, rend.h, rend.screen_buffer.lst[2*i + 0], rend.screen_buffer.lst[2*i + 1]);
        }

        // frame end 
        screen.Draw(rend.image_buffer, rend.w, rend.h);
        SDL_GL_SwapWindow(window);
    }
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
