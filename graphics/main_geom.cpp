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
    Vector3f box_center = Vector3f {0, 0, 0};
    AABox box { box_center, 0.3 };
    Camera cam { PerspectiveFrustum { 90, (float) w / h, 0.1, 20 } }; // center, dir, fov, aspect, near, far
    Vector3f cam_position { 2.2, 1.2, -5 };

    // populate vertex & line buffer
    AABoxGetCorners(box, vertex_buffer);
    AABoxGetLinesIndices(0, lines_idxbuffer);

    // build transform
    Vector3f x_hat { 1, 0, 0 };
    Vector3f y_hat { 0, 1, 0 };
    Vector3f z_hat { 0, 0, 1 };

    Matrix4f view = TransformBuild(y_hat, 0, cam_position); // TODO: LOOKAT to incorporate camera view direction
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f model = Matrix4f_Identity();
    Matrix4f mvp;


    // lookat rotations
    Vector3f d = box.center - cam_position;
    f32 d_xz_len = sqrt(d.x*d.x + d.z*d.z);
    f32 d_yz_len = sqrt(d.y*d.y + d.z*d.z);
    f32 theta = acos(d.z / d_xz_len);
    f32 phi = acos(d.z / d_yz_len);
    printf("theta %f, phi %f\n", theta * rad2deg, phi * rad2deg);
    Matrix4f R_y =  TransformBuild(y_hat, -theta);
    Matrix4f R_x = TransformBuild(x_hat, phi);
    Matrix4f lookrot = R_y * R_x;
    Matrix4f view_lookingat = view * lookrot; 


    u64 iter = 0;
    while (Running()) {
        XSleep(10);
        ClearToZeroRGBA(image_buffer, w, h);

        // setup transform: [ model -> world -> view_inv -> projection ]
        model = TransformBuild(Vector3f { 0, 1, 0 }, 0.03f * iter);
        if (iter > 50) {
            view = view_lookingat;
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
