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
    printf("Running OpenGL screen texture demo...\n");

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
    Vector3f box_position { 0, -2, 5 };
    Matrix4f box_transform = TransformBuild(y_hat, 0, box_position);
    Camera cam { PerspectiveFrustum { 90, (float) w / h, 0.1, 20 } }; // center, dir, fov, aspect, near, far
    Vector3f cam_position { 2.2, 1.2, -5 };

    // populate vertex & line buffer
    AABoxGetCorners(box, vertex_buffer);
    AABoxGetLinesIndices(0, lines_idxbuffer);


    // NOTES: 
    //  Transforms are local2world, also the view transform. However the BuildMVP() function
    //  inverts the view matrix, before multiplying into an mvp. 
    //  We are multiplying from the right, so the right-most matrix / transform applied to vertices
    //  is applied to the local coordinates.
    //  This is how the box rotation animation works currently; by applying a rotation-only matrix before
    //  the formal "box transform" is applied.


    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f view = TransformBuild(y_hat, 0, cam_position); // TODO: LOOKAT to incorporate camera view direction
    Matrix4f view_lookat = view * TransformLookRotation(box_position, cam_position); 
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f model = box_transform;
    Matrix4f mvp = BuildMVP(model, view, proj);

    u64 iter = 0;
    while (Running()) {
        XSleep(10);
        ClearToZeroRGBA(image_buffer, w, h);

        model = box_transform * TransformBuildRotateY(0.03f * iter);
        if (iter > 50) {
            view = view_lookat;
        }
        mvp = BuildMVP(model, view, proj);
        iter++;

        // project to NDC
        for (u32 i = 0; i < nvertices; ++i) {
            ndc_buffer[i] = TransformPerspective(mvp, vertex_buffer[i]);
        }

        // build screen line buffer
        u16 nlines_torender = LinesToScreen(w, h, lines_idxbuffer, nlines, ndc_buffer, lines_screen_buffer);

        // render lines
        for (u32 i = 0; i < nlines_torender; ++i) {
            DrawLineRGBA(image_buffer, w, h, lines_screen_buffer[2*i + 0], lines_screen_buffer[2*i + 1]);
        }

        // frame
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
