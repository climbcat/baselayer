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
    u8 *image = (u8*) ArenaAlloc(a, nchannels * w * h);
    SDL_Window *window = InitOGL(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);

    u16 npoints = 25;
    u16 *points = (u16*) ArenaAlloc(a, npoints * 2 * sizeof(u16));
    u16 *iter;
    while (Running()) {
        XSleep(50);
        ClearToZeroRGBA(image, w, h);
        iter = points;
        for (u16 i = 0; i < npoints; ++i) {
            *iter++ = (u16) RandDice(w);
            *iter++ = (u16) RandDice(h);
        }

        DrawTracePoints(image, w, h, points, npoints);

        screen.Draw(image, w, h);
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
