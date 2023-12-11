#ifndef __GAMELOOP_H__
#define __GAMELOOP_H__


#include <GLFW/glfw3.h>
#include <signal.h>
#include <assert.h>
#include "geometry.h"
#include "ui.h"
#include "swrender.h"


// gameloop.h: Platform code exclusively goes here, e.g. SDL / glfw / linux / win code and other APIs, but not glew / OpenGL


//
// GLFW intiialization, ctr-c exit


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


//
// GLFW input MouseTrap handlers


void MouseCursorPositionCallBack(GLFWwindow* window, double xpos, double ypos) {
    MouseTrap *mouse = (MouseTrap*) glfwGetWindowUserPointer(window);

    // NOTE: The frame-constant mouse x, y, dx and dy are handled by polling during the GameLoop
}
void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    MouseTrap *mouse = (MouseTrap*) glfwGetWindowUserPointer(window);

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouse->rheld = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse->held = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse->held = false;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        mouse->rheld = false;
    }
}
void MouseScrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
    MouseTrap *mouse = (MouseTrap*) glfwGetWindowUserPointer(window);

    mouse->wyoffset += (float) yoffset;
    if (yoffset < 0) {
        mouse->wdown = true;
    }
    else {
        mouse->wup = true;
    }
}
void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
    MouseTrap *mouse = (MouseTrap*) glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        mouse->key_esc = true;
    }
    else if (key == GLFW_KEY_SPACE) {
        mouse->key_space = true;
    }
    else if (key == GLFW_KEY_S) {
        mouse->key_s = true;
    }
}


//
// Game / frame loop glue object


struct GameLoopOne {
    u64 frameno;
    GLFWwindow* window;
    SwRenderer renderer;
    MouseTrap mouse;
    OrbitCamera cam;

    bool debug_info = false;

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
        // poll mouse for correct dx, dy
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouse.UpdatePosition((s32) xpos, (s32) ypos);

        // call key / mouse / scrool event handlers
        glfwPollEvents();
        if (debug_info) {
            mouse.PrintState();
        }

        // exit condition
        bool exit_click = glfwWindowShouldClose(window) != 0;
        bool exit_esc = mouse.key_esc;
        return !(exit_click || exit_esc);
    }
    void CycleFrame(EntitySystem *es) {
        // this frame
        cam.Update(&mouse, true);
        SwRenderFrame(&renderer, es, &cam.vp, frameno);
        glfwSwapBuffers(window);
        frameno++;

        // start of next frame
        XSleep(10);
        mouse.ResetKeyAndScrollFlags();
    }
    void Terminate() {
        glfwTerminate();
    }
};

GameLoopOne *InitGameLoopOne(MArena *a, u32 width, u32 height) {
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

    glfwSetKeyCallback(gl.window, KeyCallBack);
    glfwSetCursorPosCallback(gl.window, MouseCursorPositionCallBack);
    glfwSetMouseButtonCallback(gl.window, MouseButtonCallBack);
    glfwSetScrollCallback(gl.window, MouseScrollCallBack);

    GameLoopOne *result = (GameLoopOne*) ArenaPush(a, &gl, sizeof(GameLoopOne));
    glfwSetWindowUserPointer(result->window, &result->mouse);

    return result;
}


#endif
