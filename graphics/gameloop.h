#ifndef __GAMELOOP_H__
#define __GAMELOOP_H__

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <signal.h>


// gameloop.h: Platform code exclusively goes here, e.g. SDL / glfw / linux / win code and other APIs (but not glew / OpenGL)


//
// Game / frame loop wrapper


bool Running(GLFWwindow* window, MouseTrap *mouse); // fw-decl


struct GameLoopOne {
    u64 frameno;
    GLFWwindow* window;
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
        return Running(window, &mouse);
    }
    void CycleFrame(EntitySystem *es) {
        // this frame
        cam.Update(&mouse, true);
        SwRenderFrame(&renderer, es, &cam.vp, frameno);
        glfwSwapBuffers(window);
        frameno++;

        // start of next frame
        XSleep(10);
    }
    void Terminate() {
        glfwTerminate();
    }
};
void CtrlCHandler(int i) {
    printf("\n");
    exit(1);
}
GLFWwindow *InitGLFW(u32 width, u32 height, bool fullscreen_mode = false) {
    // glfw
    glfwInit();

    // opengl window & context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    GLFWwindow* window = glfwCreateWindow(width, height, "Point cloud viewer", NULL, NULL);
    glfwMakeContextCurrent(window);

    // glew
    glewExperimental = GL_TRUE;
    glewInit();

    // alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    return window;
}
GameLoopOne InitGameLoopOne(u32 width, u32 height) {
    GameLoopOne gl;
    gl.frameno = 0;
    gl.window = InitGLFW(width, height, false);
    gl.renderer = InitRenderer(width, height);
    gl.cam = InitOrbitCamera(gl.renderer.aspect);

    double xpos, ypos;
    glfwGetCursorPos(gl.window, &xpos, &ypos);
    s32 x = (s32) xpos;
    s32 y = (s32) ypos;

    gl.mouse = InitMouseTrap(x, y);
    return gl;
}
bool Running(GLFWwindow* window, MouseTrap *mouse) {
    // TODO: replace with glfw
    s32 x_prev = mouse->x;
    s32 y_prev = mouse->y;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouse->x = (s32) xpos;
    mouse->y = (s32) ypos;

    mouse->dx = mouse->x - x_prev;
    mouse->dy = mouse->y - y_prev;
    mouse->wup = false;
    mouse->wdown = false;
    mouse->key_space = false;
    mouse->key_s = false;

    // TODO: reimpl. with glfw event system
    /*
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
    */
    //return result;
    return glfwWindowShouldClose(window) == 0;
}


#endif
