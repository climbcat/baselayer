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


struct AABox {
    Vector3f center;
    f32 radius;
};
struct Camera {
    Vector3f position;
    Vector3f direction;
    PerspectiveFrustum frustum;
};


inline
Vector2_u16 NDC2Screen(u32 w, u32 h, Vector3f ndc) {
    Vector2_u16 pos;

    pos.x = (u32) ((ndc.x + 1) / 2 * w);
    pos.y = (u32) ((ndc.y + 1) / 2 * h);

    return pos;
}
void AABoxGetCorners(AABox box, Vector3f *dest) {
    AABox *b = &box;
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y - b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y - b->radius, b->center.z + b->radius };
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y + b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y + b->radius, b->center.z + b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y - b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y - b->radius, b->center.z + b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y + b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y + b->radius, b->center.z + b->radius };
}
u16 AABoxGetLinesIndices(u16 offset, Vector2_u16 *dest) {
    *dest++ = Vector2_u16 { (u16) (offset + 0), (u16) (offset + 1) };
    *dest++ = Vector2_u16 { (u16) (offset + 0), (u16) (offset + 2) };
    *dest++ = Vector2_u16 { (u16) (offset + 0), (u16) (offset + 4) };

    *dest++ = Vector2_u16 { (u16) (offset + 3), (u16) (offset + 1) };
    *dest++ = Vector2_u16 { (u16) (offset + 3), (u16) (offset + 2) };
    *dest++ = Vector2_u16 { (u16) (offset + 3), (u16) (offset + 7) };

    *dest++ = Vector2_u16 { (u16) (offset + 5), (u16) (offset + 1) };
    *dest++ = Vector2_u16 { (u16) (offset + 5), (u16) (offset + 4) };
    *dest++ = Vector2_u16 { (u16) (offset + 5), (u16) (offset + 7) };

    *dest++ = Vector2_u16 { (u16) (offset + 6), (u16) (offset + 2) };
    *dest++ = Vector2_u16 { (u16) (offset + 6), (u16) (offset + 4) };
    *dest++ = Vector2_u16 { (u16) (offset + 6), (u16) (offset + 7) };

    return offset + 12;
}
u16 LinesToScreen(u32 w, u32 h, Vector2_u16* lines_idxbuffer, u16 nlines, Vector3f *ndc_buffer, Vector2_u16* lines_screen_buffer) {

    u16 nlines_torender = 0;
    u16 idx = 0;
    for (u32 i = 0; i < nlines; ++i) {
        Vector2_u16 line = lines_idxbuffer[i];
        Vector2_u16 a = NDC2Screen(w, h, ndc_buffer[line.x]);
        Vector2_u16 b = NDC2Screen(w, h, ndc_buffer[line.y]);

        // TODO: crop to NDC box
        if (Cull(a.x, a.y, w, h) || Cull(b.x, b.y, w, h)) {
            continue;
        }

        lines_screen_buffer[idx++] = a;
        lines_screen_buffer[idx++] = b;
        nlines_torender++;
    }
    return nlines_torender;
}


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
    AABox box { Vector3f {0, 1, 4}, 0.3 };
    Camera cam { Vector3f {1, 1, -1}, Vector3f {0, 0, 1}, PerspectiveFrustum { 90, 1, 0.1, 6 } }; // center, dir, fov, aspect, near, far

    // populate vertex & line buffer
    AABoxGetCorners(box, vertex_buffer);
    AABoxGetLinesIndices(0, lines_idxbuffer);

    // build transform
    Matrix4f view = TransformBuildTranslationOnly(cam.position); // TODO: LOOKAT to incorporate camera view direction
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f model = Matrix4f_Identity(); // TODO: apply some interesting transform
    Matrix4f mvp = proj * TransformGetInverse( view ) * model;

    u16 *iter;
    while (Running()) {
        XSleep(33);
        ClearToZeroRGBA(image_buffer, w, h);

        // TODO: update model transform

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
