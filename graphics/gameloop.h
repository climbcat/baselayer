#ifndef __GAMELOOP_H__
#define __GAMELOOP_H__


// gameloop.h: Platform code exclusively goes here, e.g. SDL / glfw / linux / win code and other APIs (but not glew / OpenGL)


//
// Game / frame loop wrapper


bool Running(MouseTrap *mouse); // fw-decl


struct GameLoopOne {
    u64 frameno;
    SDL_Window *window; // <- TODO: replace with glfw
    SwRenderer renderer;
    MouseTrap mouse;
    OrbitCamera cam;

    // as pointer
    inline
    SwRenderer *GetRenderer() {
        return &renderer;
    }
    inline
    MouseTrap *GetMouseTrap()  {
        return &mouse;
    }
    inline
    OrbitCamera *GetOrbitCam() {
        return &cam;
    }
    bool GameLoopRunning() {
        return Running(&mouse);
    }
    void CycleFrame(EntitySystem *es) {
        // this frame
        cam.Update(&mouse, true);
        SwRenderFrame(&renderer, es, &cam.vp, frameno);
        SDL_GL_SwapWindow(window);
        frameno++;

        // start of next frame
        XSleep(10);
    }
};
void CtrlCHandler(int i) {
    printf("\n");
    exit(1);
}
SDL_Window *InitSDL(u32 width, u32 height, bool fullscreen_mode = false) {
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
GameLoopOne InitGameLoopOne(u32 width, u32 height) {
    GameLoopOne gl;
    gl.frameno = 0;
    gl.window = InitSDL(width, height, false);
    gl.renderer = InitRenderer(width, height);
    gl.cam = InitOrbitCamera(gl.renderer.aspect);
    int x, y;
    SDL_GetMouseState(&x, &y);
    gl.mouse = InitMouseTrap(x, y);
    return gl;
}
bool Running(MouseTrap *mouse) {
    // TODO: replace with glfw
    s32 x_prev = mouse->x;
    s32 y_prev = mouse->y;
    SDL_GetMouseState(&mouse->x, &mouse->y);
    mouse->dx = mouse->x - x_prev;
    mouse->dy = mouse->y - y_prev;
    mouse->wup = false;
    mouse->wdown = false;
    mouse->key_space = false;
    mouse->key_s = false;

    bool result = true;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) {
            result = false;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
            result = false;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) {
            mouse->key_space = true;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_s) {
            mouse->key_s = true;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            mouse->held = true;
        }
        else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            mouse->held = false;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
            mouse->rheld = true;
        }
        else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
            mouse->rheld = false;
        }
        else if (event.type == SDL_MOUSEWHEEL) {
            if (event.wheel.y > 0) {
                mouse->wup = true;
            }
            else if (event.wheel.y < 0) {
                mouse->wdown = true;
            }
        }
    }
    return result;
}


#endif
