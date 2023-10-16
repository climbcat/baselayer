#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../baselayer.h"
#include "geometry.h"
#include "shaders.h"
#include "test.cpp"

#define DEFAULT_WINDOW_WIDTH 1440
#define DEFAULT_WINDOW_HEIGHT 800
#include <signal.h>

void CtrlCHandler(int i) {
    printf("\n");
    exit(1);
}

SDL_Window *InitOGL(u32 width, u32 height, bool fullscreen_mode = false) {
    // init SDL / OS window manager, and OpenGL context
    SDL_Init(SDL_INIT_VIDEO);
    if (fullscreen_mode) {
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        width = dm.w;
        height = dm.h;
    }
    // override SDL's hijacking signals
    signal(SIGINT, CtrlCHandler);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_Window *window = SDL_CreateWindow("Point cloud viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;
    glewInit();

    // fullscreen mode switch
    if (fullscreen_mode) {
        printf("Entering full screen mode: %d %d\n", width, height);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }

    // alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    return window;
}

bool Running() {
    SDL_Event event;
    //SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) {
            return false;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
            return false;
        }
    }
    return true;
}

void RunProgram() {
    printf("Running OpenGL screen texture demo...\n");

    // prep random image
    u32 w = 640;
    u32 h = 480;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    u32 nchannels = 4;
    u8 *image = (u8*) ArenaAlloc(a, nchannels * w * h);
    RandInit();

    // OGL window & sharder
    SDL_Window *window = InitOGL(w, h);
    ScreenQuadTextureProgram sqprog;
    sqprog.Init(image, w, h);

    while (Running()) {
        usleep(10000);

        u32 pix_idx, r, g, b;
        for (u32 i = 0; i < h; ++i) {
            for (u32 j = 0; j < w; ++j) {
                pix_idx = j + i * w;
                r = RandIntMax(255);
                g = RandIntMax(255);
                b = RandIntMax(255);
                image[nchannels * pix_idx + 0] = r;
                image[nchannels * pix_idx + 1] = g;
                image[nchannels * pix_idx + 2] = b;
                image[nchannels * pix_idx + 3] = 255;
            }
        }

        sqprog.Draw(image, w, h);
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
}
