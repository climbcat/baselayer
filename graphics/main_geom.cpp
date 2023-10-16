#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../baselayer.h"
#include "geometry.h"
#include "test.cpp"

#define DEFAULT_WINDOW_WIDTH 1440
#define DEFAULT_WINDOW_HEIGHT 800
#include <signal.h>

void CtrlCHandler(int i) {
    printf("\n");
    exit(1);
}

void InitOGL(bool fullscreen_mode = false) {
    u32 window_width = DEFAULT_WINDOW_WIDTH;
    u32 window_height = DEFAULT_WINDOW_HEIGHT;

    // init SDL / OS window manager, and OpenGL context
    SDL_Init(SDL_INIT_VIDEO);
    if (fullscreen_mode) {
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        window_width = dm.w;
        window_height = dm.h;
    }
    // override SDL's hijacking signals
    signal(SIGINT, CtrlCHandler);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_Window *window = SDL_CreateWindow("Point cloud viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width,
        window_height,
        SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;
    glewInit();

    if (fullscreen_mode) {
        printf("Entering full screen mode: %d %d\n", window_width, window_height);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }

    // alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}


void RunProgram() {
    InitOGL();

    while (true) {
        printf("gameplay loop ...\n");
        usleep(1000000);
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
