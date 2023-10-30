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

    u32 w = 1280;
    u32 h = 800;
    float aspect = (float) w / h;
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    SDL_Window *window = InitOGL(w, h);

    // image buffer
    u8 *image_buffer = (u8*) ArenaAlloc(a, nchannels * w * h);
    ScreenQuadTextureProgram screen;
    screen.Init(image_buffer, w, h);

    // pipeline buffers
    List<Vector3f> vertex_buffer = InitList<Vector3f>(a, 1000);
    List<Vector2_u16> index_buffer = InitList<Vector2_u16>(a, 1000);
    List<Vector3f> ndc_buffer = InitList<Vector3f>(a, 1000);
    List<Vector2_u16> screen_buffer = InitList<Vector2_u16>(a, 1000);

    // entities
    AABox box = InitAABox({ 0, -0.7, 0.7 }, 0.3);
    CoordAxes axes = InitCoordAxes();
    AABoxGetVerticesAndIndices(box, &vertex_buffer, &index_buffer);
    CoordAxesGetVerticesAndIndices(axes, &vertex_buffer, &index_buffer);

    // camera
    OrbitCamera cam = InitOrbitCamera(aspect);

    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f view, model, mvp;



    // NOTE: noticed that upper-left-corner constitutes screen 0,0, which means we much 
    //  want to invert the y-axis back from whence it came :>


    u64 iter = 0;
    MouseTrap mouse = InitMouseTrap();
    while (Running(&mouse)) {
        XSleep(10);
        ClearToZeroRGBA(image_buffer, w, h);
        screen_buffer.len = 0;

        // box animation
        model = box.transform * TransformBuildRotateY(0.03f * iter);

        // orbit
        cam.Update(&mouse);
        mvp = TransformBuildMVP(model, cam.view, proj);
        iter++;

        // render
        for (u32 i = 0; i < vertex_buffer.len; ++i) {
            ndc_buffer.lst[i] = TransformPerspective(mvp, vertex_buffer.lst[i]);
        }
        ndc_buffer.len = vertex_buffer.len;

        u16 nlines_torender = LinesToScreen(w, h, &index_buffer, &ndc_buffer, &screen_buffer);
        for (u32 i = 0; i < screen_buffer.len / 2; ++i) {
            RenderLineRGBA(image_buffer, w, h, screen_buffer.lst[2*i + 0], screen_buffer.lst[2*i + 1]);
        }
        screen.Draw(image_buffer, w, h);
        SDL_GL_SwapWindow(window);
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
