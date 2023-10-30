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
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    SDL_Window *window = InitOGL(w, h);

    // image buffer
    u8 *image_buffer = (u8*) ArenaAlloc(a, nchannels * w * h);
    ScreenQuadTextureProgram screen;
    screen.Init(image_buffer, w, h);

    // pipeline buffers
    Vector3f *vertex_buffer = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * 100);
    u32 nvertices = 8;
    Vector3f *ndc_buffer = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * 100);
    Vector2_u16 *lines_idxbuffer = (Vector2_u16*) ArenaAlloc(a, sizeof(Vector2_u16) * 1000);
    u32 nlines = 12;
    Vector2_u16 *lines_screen_buffer = (Vector2_u16*) ArenaAlloc(a, sizeof(Vector2_u16) * 1000);

    // entities
    Vector3f box_center = Vector3f {0, 0, 0}; // local coords
    AABox box { box_center, 0.3 };
    //Vector3f box_position { 0, -2, 5 };
    Vector3f box_position { 0, 0, 0 };
    Matrix4f box_transform = TransformBuild(y_hat, 0, box_position);
    Camera cam { PerspectiveFrustum { 90, (float) w / h, 0.1, 20 } }; // center, dir, fov, aspect, near, far
    //Vector3f cam_position { 0, 1, 8 };
    //Vector3f cam_position { 0, 1, -8 };
    //Vector3f cam_position { 8, 1, 1 };
    Vector3f cam_position { -8, 1, 1 };

    // populate vertex & line buffer
    AABoxGetCorners(box, vertex_buffer);
    AABoxGetLinesIndices(0, lines_idxbuffer);

    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f view = TransformBuild(y_hat, 0, cam_position) * TransformBuildLookRotationYUp(box_position, cam_position); 
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f model = box_transform;
    Matrix4f mvp;


    // NOTE: noticed that upper-left-corner constitutes screen 0,0, which means we much 
    //  want to invert the y-axis back from whence it came :>


    // orbit camera params 
    float theta = 85;
    float phi = 0;
    float radius = 8;

    u64 iter = 0;
    MouseTrap mouse = InitMouseTrap();
    while (Running(&mouse)) {
        XSleep(10);
        ClearToZeroRGBA(image_buffer, w, h);

        // contact the mouse
        printf("left: %d right: %d x: %d y: %d dx: %d dy: %d\n", mouse.held, mouse.rheld, mouse.x, mouse.y, mouse.dx, mouse.dy);

        // model transforms
        model = box_transform * TransformBuildRotateY(0.03f * iter);

        // orbitcam stuff
        cam_position = SphericalCoordsY(0.2*(sin(iter*3 * deg2rad) - 2) + theta * deg2rad, phi * deg2rad, radius);
        view = TransformBuild(y_hat, 0, cam_position) * TransformBuildLookRotationYUp(box_center, cam_position);
        mvp = TransformBuildMVP(model, view, proj);
        iter++;

        // render
        for (u32 i = 0; i < nvertices; ++i) {
            ndc_buffer[i] = TransformPerspective(mvp, vertex_buffer[i]);
        }
        u16 nlines_torender = LinesToScreen(w, h, lines_idxbuffer, nlines, ndc_buffer, lines_screen_buffer);
        for (u32 i = 0; i < nlines_torender; ++i) {
            RenderLineRGBA(image_buffer, w, h, lines_screen_buffer[2*i + 0], lines_screen_buffer[2*i + 1]);
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
