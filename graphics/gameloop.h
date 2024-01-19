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
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        mouse->key_space = true;
    }

    else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        mouse->key_s = true;
    }
    else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        mouse->key_n = true;
    }
    else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        mouse->key_h = true;
    }

    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        mouse->key_left = true;
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        mouse->key_right = true;
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        mouse->key_up = true;
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        mouse->key_down = true;
    }

    else if (key == GLFW_KEY_0 && action == GLFW_PRESS) { 
        mouse->key_digit = 0;
    }
    else if (key == GLFW_KEY_1 && action == GLFW_PRESS) { 
        mouse->key_digit = 1;
    }
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS) { 
        mouse->key_digit = 2;
    }
    else if (key == GLFW_KEY_3 && action == GLFW_PRESS) { 
        mouse->key_digit = 3;
    }
    else if (key == GLFW_KEY_4 && action == GLFW_PRESS) { 
        mouse->key_digit = 4;
    }
    else if (key == GLFW_KEY_5 && action == GLFW_PRESS) { 
        mouse->key_digit = 5;
    }
    else if (key == GLFW_KEY_6 && action == GLFW_PRESS) { 
        mouse->key_digit = 6;
    }
    else if (key == GLFW_KEY_7 && action == GLFW_PRESS) { 
        mouse->key_digit = 7;
    }
    else if (key == GLFW_KEY_8 && action == GLFW_PRESS) { 
        mouse->key_digit = 8;
    }
    else if (key == GLFW_KEY_9 && action == GLFW_PRESS) { 
        mouse->key_digit = 9;
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

    SwRenderer *GetRenderer() {
        return &renderer;
    }
    MouseTrap *GetMouseTrap()  {
        return &mouse;
    }
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
    void JustRun(EntitySystem *es) {
        while (GameLoopRunning()) {
            CycleFrame(es);
        }
        Terminate();
    }
    void Terminate() {
        glfwTerminate();
    }
};
void GameLoopJustRun(GameLoopOne *game, EntitySystem *es) {
    game->JustRun(es);
}

static GameLoopOne _g_loop;
static GameLoopOne *g_loop;
GameLoopOne *InitGameLoopOne(MArena *a = NULL, u32 width = 1280, u32 height = 800) {
    assert(g_loop == NULL && "singleton assert");
    g_loop = &_g_loop;

    g_loop->frameno = 0;
    g_loop->window = InitGLFW(width, height, false);
    g_loop->renderer = InitRenderer(width, height);
    g_loop->cam = InitOrbitCamera(g_loop->renderer.aspect);

    double xpos, ypos;
    glfwGetCursorPos(g_loop->window, &xpos, &ypos);
    s32 x = (s32) xpos;
    s32 y = (s32) ypos;
    g_loop->mouse = InitMouseTrap(x, y);

    glfwSetKeyCallback(g_loop->window, KeyCallBack);
    glfwSetCursorPosCallback(g_loop->window, MouseCursorPositionCallBack);
    glfwSetMouseButtonCallback(g_loop->window, MouseButtonCallBack);
    glfwSetScrollCallback(g_loop->window, MouseScrollCallBack);
    glfwSetWindowUserPointer(g_loop->window, &g_loop->mouse);

    return g_loop;
}


#endif
