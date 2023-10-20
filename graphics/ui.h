#ifndef __UI_H__
#define __UI_H__


#define DEFAULT_WINDOW_WIDTH 1440
#define DEFAULT_WINDOW_HEIGHT 800


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


#endif