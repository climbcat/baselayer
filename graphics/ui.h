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
struct MouseTrap {
    s32 x = 0;
    s32 y = 0;
    s32 dx = 0;
    s32 dy = 0;
    bool held;
    bool rheld;
    bool wup;
    bool wdown;
};
MouseTrap InitMouseTrap() {
    MouseTrap m;
    SDL_GetMouseState(&m.x, &m.y);
    m.held = false;
    m.rheld = false;
    m.wup = true;
    m.wdown = true;
    return m;
}
bool Running(MouseTrap *mouse) {
    s32 x_prev = mouse->x;
    s32 y_prev = mouse->y;
    SDL_GetMouseState(&mouse->x, &mouse->y);
    mouse->dx = mouse->x - x_prev;
    mouse->dy = mouse->y - y_prev;
    mouse->wup = false;
    mouse->wdown = false;

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


struct OrbitCamera {
    PerspectiveFrustum frustum;
    Vector3f center;
    float theta;
    float phi;
    float radius;
    float mouse2rot = 0.4;
    float mouse2pan = 0.01;
    Matrix4f view;

    static float ClampTheta(float theta_degs, float min = 0.0001f, float max = 180 - 0.0001f) {
        float clamp_up = MinF32(theta_degs, max);
        float result = MaxF32(clamp_up, min);
        return result;
    }
    void Update(MouseTrap *m, bool invert_x) {
        float sign_x = 1;
        if (invert_x) {
            sign_x = - 1;
        }

        if (m->held) {
            // orbit
            theta = OrbitCamera::ClampTheta(theta - m->dy * mouse2rot);
            phi += sign_x * m->dx * mouse2rot;
        }
        else if (m->wdown) {
            radius *= 1.1;
        }
        else if (m->wup) {
            radius /= 1.1;
        }
        else if (m->rheld) {
            // pan
            Vector3f forward = - SphericalCoordsY(theta*deg2rad, phi*deg2rad, radius);
            forward.Normalize();
            Vector3f left = y_hat.Cross(forward);
            left.Normalize();
            Vector3f right = - left;
            Vector3f up = forward.Cross(left);
            up.Normalize();
            center = center + mouse2pan * m->dx * right;
            center = center + mouse2pan * m->dy * up;
        }

        // build orbit camp transform
        view = TransformBuildOrbitCam(center, theta, phi, radius);
    }
};
OrbitCamera InitOrbitCamera(float aspect) {
    OrbitCamera cam { PerspectiveFrustum { 90, aspect, 0.1, 20 } };
    cam.center = Vector3f_Zero();
    cam.theta = 60;
    cam.phi = 35;
    cam.radius = 8;
    cam.view = Matrix4f_Identity();
    return cam;
}


#endif
