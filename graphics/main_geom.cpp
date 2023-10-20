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

    // setup image data
    u32 w = 1280;
    u32 h = 800;
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    u8 *image = (u8*) ArenaAlloc(a, nchannels * w * h);

    // OGL window & sharder
    SDL_Window *window = InitOGL(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);
    
    while (Running()) {
        XSleep(50);

        DrawLineRGBA(image, w, h, 15, 200, 500, 500);
        DrawLineRGBA(image, w, h, 1000, 10, 15, 500);
        DrawLineRGBA(image, w, h, 400, 200, 900, 200);
        DrawLineRGBA(image, w, h, 800, 10, 800, 500);
        DrawLineRGBA(image, w, h, 920, 120, 900, 790);
        DrawLineRGBA(image, w, h, 920, 120, 950, 790);

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
