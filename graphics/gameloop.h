#ifndef __GAMELOOP_H__
#define __GAMELOOP_H__


#include <GLFW/glfw3.h>
#include <signal.h>
#include <assert.h>
#include "geometry.h"
#include "ui.h"
#include "sprite.h"
#include "swrender.h"


// gameloop.h: Platform code exclusively goes here, e.g. SDL / glfw / linux / win code and other APIs, but not glew / OpenGL


//
// GLFW intiialization, ctr-c exit


void CtrlCHandler(int i) {
    printf("\n");
    exit(1);
}
GLFWwindow *InitGLFW(u32 width, u32 height, const char *title, bool fullscreen_mode = false) {
    // glfw
    glfwInit();

    // opengl window & context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
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
// Game / frame loop glue object


#define GAMELOOPONE_EVENT_QUEUE_CAPACITY 100
struct GameLoopOne {
    u64 frameno;
    GLFWwindow* window;
    SwRenderer renderer;
    MouseTrap mouse;
    OrbitCamera cam;

    UiEvent events_mem[GAMELOOPONE_EVENT_QUEUE_CAPACITY];
    u32 event_queue_capacity;
    List<UiEvent> event_queue;
    void PushEvent(UiEvent event) {
        assert(event_queue.len < event_queue_capacity && "game loop event queue capacity exceeded");

        event_queue.Add(event);
    }
    UiEvent *PopEvent() {
        UiEvent *pop = NULL;
        if (event_queue.len > 0) {
            --event_queue.len;
            pop = event_queue.lst + event_queue.len;
        }
        return pop;
    }
    bool GetKey(Key k) {
        bool result = glfwGetKey(window, k) == GLFW_PRESS;
        return result;
    }


    SwRenderer *GetRenderer() {
        if (renderer.initialized) {
            return &renderer;
        }
        else {
            return NULL;
        }
    }
    MouseTrap *GetMouseTrap()  {
        return &mouse;
    }
    OrbitCamera *GetOrbitCam() {
        return &cam;
    }

    // TODO: rename GameLoopRunning to GameLoopFrameBegin()
    bool _is_running;
    bool _exit_request;
    bool GameLoopRunning() {
        // poll mouse for correct dx, dy
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouse.UpdateFrameMouseState((s32) xpos, (s32) ypos);

        // call key / mouse / scrool event handlers
        glfwPollEvents();

        bool exit_esc = false;
        while (UiEvent *e = PopEvent()) {
            // notify mouse
            mouse.Update(*e);

            // notify exit condition
            if (e->key == OUR_GLFW_KEY_ESCAPE && e->action == OUR_GLFW_PRESS) {
                exit_esc = true;
            }

            // TODO: here we should update all entities
        }

        // exit condition
        bool exit_click = glfwWindowShouldClose(window) != 0;
        _is_running = !(exit_click || exit_esc || _exit_request);
        return _is_running;
    }
    bool GetIsRunning() {
        return _is_running;
    }

    // TODO: Rename to GameLoopFrameEnd()
    void CycleFrame(EntitySystem *es) {
        cam.Update(mouse);
        mouse.FrameEnd(frameno);

        SwRenderFrame(&renderer, es, &cam.vp, frameno);
        glfwSwapBuffers(window);

        frameno++;
        XSleep(10);
    }
    void FrameStart2D(Color clear = { RGBA_BLACK }) {
        Color *img = (Color *) renderer.image_buffer;
        for (u32 i = 0; i < renderer.w * renderer.h; ++i) {
            img[i] = clear;
        }
        SR_Clear();
    }
    void FrameEnd2D() {
        mouse.FrameEnd(frameno);

        SR_Render();
        ImageBufferDrawAndSwap();

        frameno++;
        XSleep(10);
    }
    void FrameEnd2DAnd3D(EntitySystem *es) {
        cam.Update(mouse);
        mouse.FrameEnd(frameno);

        SwRenderFrame(&renderer, es, &cam.vp, frameno);
        SR_Render();
        ImageBufferDrawAndSwap();

        frameno++;
        XSleep(10);
    }
    void JustRun(EntitySystem *es) {
        while (GameLoopRunning()) {
            CycleFrame(es);
        }
        Terminate();
    }
    void JustShowBuffer() {
        renderer.screen.Draw(renderer.image_buffer, renderer.w, renderer.h);
        glfwSwapBuffers(window);

        while (GameLoopRunning()) {
            XSleep(10);
        }
        Terminate();
    }
    void ImageBufferClear() {
        _memzero(renderer.image_buffer, 4*renderer.w * renderer.h);
    }
    void ImageBufferDrawAndSwap() {
        renderer.screen.Draw(renderer.image_buffer, renderer.w, renderer.h);
        glfwSwapBuffers(window);
    }

    void Exit() {
        _exit_request = true;
    }
    void Terminate() {
        glfwTerminate();
    }
    void Close() {
        glfwDestroyWindow(window);
    }
};
void MouseCursorPositionCallBack(GLFWwindow* window, double xpos, double ypos) {
    // empty
}
void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    GameLoopOne *game_loop = (GameLoopOne*) glfwGetWindowUserPointer(window);
    UiEvent event = InitUiEvent(button, action, mods, 0);
    game_loop->PushEvent(event);
}
void MouseScrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
    GameLoopOne *game_loop = (GameLoopOne*) glfwGetWindowUserPointer(window);
    UiEvent event = InitUiEvent(-1, -1, -1, yoffset);
    game_loop->PushEvent(event);
}
void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GameLoopOne *game_loop = (GameLoopOne*) glfwGetWindowUserPointer(window);
    UiEvent event = InitUiEvent(key, action, mods, 0);
    game_loop->PushEvent(event);
}
void GameLoopJustRun(GameLoopOne *loop, EntitySystem *es) {
    loop->JustRun(es);
}

static GameLoopOne _g_gameloop;
static GameLoopOne *g_gameloop;
static MouseTrap *g_mouse;
GameLoopOne *InitGameLoopOne(u32 width = 1280, u32 height = 800, const char *window_title = "") {
    if (g_gameloop != NULL) {
        FreeRenderer(&g_gameloop->renderer);
        _memzero(g_gameloop, sizeof(GameLoopOne));
    }

    g_gameloop = &_g_gameloop;
    g_gameloop->frameno = 0;
    g_gameloop->window = InitGLFW(width, height, window_title, false);
    g_gameloop->renderer = InitRenderer(width, height);
    g_gameloop->cam = InitOrbitCamera(g_gameloop->renderer.aspect);

    double xpos, ypos;
    glfwGetCursorPos(g_gameloop->window, &xpos, &ypos);
    g_gameloop->mouse = InitMouseTrap((s32) xpos, (s32) ypos);

    g_gameloop->event_queue = List<UiEvent>{ &g_gameloop->events_mem[0], 0 };
    g_gameloop->event_queue_capacity = GAMELOOPONE_EVENT_QUEUE_CAPACITY;

    glfwSetKeyCallback(g_gameloop->window, KeyCallBack);
    glfwSetCursorPosCallback(g_gameloop->window, MouseCursorPositionCallBack);
    glfwSetMouseButtonCallback(g_gameloop->window, MouseButtonCallBack);
    glfwSetScrollCallback(g_gameloop->window, MouseScrollCallBack);
    glfwSetWindowUserPointer(g_gameloop->window, g_gameloop);

    g_mouse = g_gameloop->GetMouseTrap();

    return g_gameloop;
}
MouseTrap *GetMouse() {
    assert(g_mouse != NULL);
    return g_mouse;
}


#endif
