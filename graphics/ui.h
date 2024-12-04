#ifndef __UI_H__
#define __UI_H__


#include "../baselayer.h"
#include "geometry.h"


#define DEFAULT_WINDOW_WIDTH 1440
#define DEFAULT_WINDOW_HEIGHT 800


#define OUR_GLFW_RELEASE                0
#define OUR_GLFW_PRESS                  1
#define OUR_GLFW_REPEAT                 2

#define OUR_GLFW_KEY_UNKNOWN            -1
#define OUR_GLFW_KEY_SPACE              32
#define OUR_GLFW_KEY_APOSTROPHE         39
#define OUR_GLFW_KEY_COMMA              44
#define OUR_GLFW_KEY_MINUS              45
#define OUR_GLFW_KEY_PERIOD             46
#define OUR_GLFW_KEY_SLASH              47
#define OUR_GLFW_KEY_0                  48
#define OUR_GLFW_KEY_1                  49
#define OUR_GLFW_KEY_2                  50
#define OUR_GLFW_KEY_3                  51
#define OUR_GLFW_KEY_4                  52
#define OUR_GLFW_KEY_5                  53
#define OUR_GLFW_KEY_6                  54
#define OUR_GLFW_KEY_7                  55
#define OUR_GLFW_KEY_8                  56
#define OUR_GLFW_KEY_9                  57
#define OUR_GLFW_KEY_SEMICOLON          59
#define OUR_GLFW_KEY_EQUAL              61
#define OUR_GLFW_KEY_A                  65
#define OUR_GLFW_KEY_B                  66
#define OUR_GLFW_KEY_C                  67
#define OUR_GLFW_KEY_D                  68
#define OUR_GLFW_KEY_E                  69
#define OUR_GLFW_KEY_F                  70
#define OUR_GLFW_KEY_G                  71
#define OUR_GLFW_KEY_H                  72
#define OUR_GLFW_KEY_I                  73
#define OUR_GLFW_KEY_J                  74
#define OUR_GLFW_KEY_K                  75
#define OUR_GLFW_KEY_L                  76
#define OUR_GLFW_KEY_M                  77
#define OUR_GLFW_KEY_N                  78
#define OUR_GLFW_KEY_O                  79
#define OUR_GLFW_KEY_P                  80
#define OUR_GLFW_KEY_Q                  81
#define OUR_GLFW_KEY_R                  82
#define OUR_GLFW_KEY_S                  83
#define OUR_GLFW_KEY_T                  84
#define OUR_GLFW_KEY_U                  85
#define OUR_GLFW_KEY_V                  86
#define OUR_GLFW_KEY_W                  87
#define OUR_GLFW_KEY_X                  88
#define OUR_GLFW_KEY_Y                  89
#define OUR_GLFW_KEY_Z                  90
#define OUR_GLFW_KEY_LEFT_BRACKET       91
#define OUR_GLFW_KEY_BACKSLASH          92
#define OUR_GLFW_KEY_RIGHT_BRACKET      93
#define OUR_GLFW_KEY_GRAVE_ACCENT       96
#define OUR_GLFW_KEY_WORLD_1            161
#define OUR_GLFW_KEY_WORLD_2            162
#define OUR_GLFW_KEY_ESCAPE             256
#define OUR_GLFW_KEY_ENTER              257
#define OUR_GLFW_KEY_TAB                258
#define OUR_GLFW_KEY_BACKSPACE          259
#define OUR_GLFW_KEY_INSERT             260
#define OUR_GLFW_KEY_DELETE             261
#define OUR_GLFW_KEY_RIGHT              262
#define OUR_GLFW_KEY_LEFT               263
#define OUR_GLFW_KEY_DOWN               264
#define OUR_GLFW_KEY_UP                 265
#define OUR_GLFW_KEY_PAGE_UP            266
#define OUR_GLFW_KEY_PAGE_DOWN          267
#define OUR_GLFW_KEY_HOME               268
#define OUR_GLFW_KEY_END                269
#define OUR_GLFW_KEY_CAPS_LOCK          280
#define OUR_GLFW_KEY_SCROLL_LOCK        281
#define OUR_GLFW_KEY_NUM_LOCK           282
#define OUR_GLFW_KEY_PRINT_SCREEN       283
#define OUR_GLFW_KEY_PAUSE              284
#define OUR_GLFW_KEY_F1                 290
#define OUR_GLFW_KEY_F2                 291
#define OUR_GLFW_KEY_F3                 292
#define OUR_GLFW_KEY_F4                 293
#define OUR_GLFW_KEY_F5                 294
#define OUR_GLFW_KEY_F6                 295
#define OUR_GLFW_KEY_F7                 296
#define OUR_GLFW_KEY_F8                 297
#define OUR_GLFW_KEY_F9                 298
#define OUR_GLFW_KEY_F10                299
#define OUR_GLFW_KEY_F11                300
#define OUR_GLFW_KEY_F12                301
#define OUR_GLFW_KEY_F13                302
#define OUR_GLFW_KEY_F14                303
#define OUR_GLFW_KEY_F15                304
#define OUR_GLFW_KEY_F16                305
#define OUR_GLFW_KEY_F17                306
#define OUR_GLFW_KEY_F18                307
#define OUR_GLFW_KEY_F19                308
#define OUR_GLFW_KEY_F20                309
#define OUR_GLFW_KEY_F21                310
#define OUR_GLFW_KEY_F22                311
#define OUR_GLFW_KEY_F23                312
#define OUR_GLFW_KEY_F24                313
#define OUR_GLFW_KEY_F25                314
#define OUR_GLFW_KEY_KP_0               320
#define OUR_GLFW_KEY_KP_1               321
#define OUR_GLFW_KEY_KP_2               322
#define OUR_GLFW_KEY_KP_3               323
#define OUR_GLFW_KEY_KP_4               324
#define OUR_GLFW_KEY_KP_5               325
#define OUR_GLFW_KEY_KP_6               326
#define OUR_GLFW_KEY_KP_7               327
#define OUR_GLFW_KEY_KP_8               328
#define OUR_GLFW_KEY_KP_9               329
#define OUR_GLFW_KEY_KP_DECIMAL         330
#define OUR_GLFW_KEY_KP_DIVIDE          331
#define OUR_GLFW_KEY_KP_MULTIPLY        332
#define OUR_GLFW_KEY_KP_SUBTRACT        333
#define OUR_GLFW_KEY_KP_ADD             334
#define OUR_GLFW_KEY_KP_ENTER           335
#define OUR_GLFW_KEY_KP_EQUAL           336
#define OUR_GLFW_KEY_LEFT_SHIFT         340
#define OUR_GLFW_KEY_LEFT_CONTROL       341
#define OUR_GLFW_KEY_LEFT_ALT           342
#define OUR_GLFW_KEY_LEFT_SUPER         343
#define OUR_GLFW_KEY_RIGHT_SHIFT        344
#define OUR_GLFW_KEY_RIGHT_CONTROL      345
#define OUR_GLFW_KEY_RIGHT_ALT          346
#define OUR_GLFW_KEY_RIGHT_SUPER        347
#define OUR_GLFW_KEY_MENU               348
#define OUR_GLFW_KEY_LAST               OUR_GLFW_KEY_MENU

#define OUR_GLFW_MOUSE_BUTTON_1         0
#define OUR_GLFW_MOUSE_BUTTON_2         1
#define OUR_GLFW_MOUSE_BUTTON_3         2
#define OUR_GLFW_MOUSE_BUTTON_4         3
#define OUR_GLFW_MOUSE_BUTTON_5         4
#define OUR_GLFW_MOUSE_BUTTON_6         5
#define OUR_GLFW_MOUSE_BUTTON_7         6
#define OUR_GLFW_MOUSE_BUTTON_8         7
#define OUR_GLFW_MOUSE_BUTTON_LAST      OUR_GLFW_MOUSE_BUTTON_8
#define OUR_GLFW_MOUSE_BUTTON_LEFT      OUR_GLFW_MOUSE_BUTTON_1
#define OUR_GLFW_MOUSE_BUTTON_RIGHT     OUR_GLFW_MOUSE_BUTTON_2
#define OUR_GLFW_MOUSE_BUTTON_MIDDLE    OUR_GLFW_MOUSE_BUTTON_3


typedef s32 KeyAction;
typedef s32 Key;
typedef s32 KeyMods;


struct UiEvent {
    Key key;
    KeyAction action;
    KeyMods mods;
    double mwheel_y_delta;
};
static UiEvent _zero_uievent;
UiEvent InitUiEvent(Key key, KeyAction action, KeyMods mods, double mwheel_y_delta) {
    UiEvent event;
    event.key = key;
    event.action = action;
    event.mods = mods;
    event.mwheel_y_delta = mwheel_y_delta;

    return event;
}


struct MouseTrap {
    u64 frameno;

    // state since last frame
    s32 x;
    s32 y;
    s32 dx;
    s32 dy;

    // state since last event
    bool l;
    bool r;
    bool m;
    u64 ldo_fn;
    u64 lup_fn;
    s32 dl;
    s32 dr;
    s32 dm;

    double mwheel_y_delta;
    Key last_keypress_frame;

    void FrameEnd(u64 frameno) {
        this->frameno = frameno;
        mwheel_y_delta = 0;
        last_keypress_frame = 0;

        dl = 0;
        dr = 0;
        dm = 0;
    }
    inline
    void UpdateFrameMouseState(s32 xpos, s32 ypos) {
        dx = xpos - x;
        dy = ypos - y;
        x = xpos;
        y = ypos;
    }
    void Update(UiEvent event) {
        bool ldown =
            (event.key == OUR_GLFW_MOUSE_BUTTON_LEFT) &&
            (event.action == OUR_GLFW_PRESS);
        bool lup =
            (event.key == OUR_GLFW_MOUSE_BUTTON_LEFT) &&
            (event.action == OUR_GLFW_RELEASE);
        bool rdown =
            (event.key == OUR_GLFW_MOUSE_BUTTON_RIGHT) &&
            (event.action == OUR_GLFW_PRESS);
        bool rup =
            (event.key == OUR_GLFW_MOUSE_BUTTON_RIGHT) &&
            (event.action == OUR_GLFW_RELEASE);
        bool mdown =
            (event.key == OUR_GLFW_MOUSE_BUTTON_MIDDLE) &&
            (event.action == OUR_GLFW_PRESS);
        bool mup =
            (event.key == OUR_GLFW_MOUSE_BUTTON_MIDDLE) &&
            (event.action == OUR_GLFW_RELEASE);

        ldo_fn += ldown * (frameno - ldo_fn);
        lup_fn += lup * (frameno - lup_fn);

        dl = (s32) lup - (s32) ldown;
        dr = (s32) rup - (s32) rdown;
        dm = (s32) mup - (s32) mdown;

        l |= (dl == -1); // transition down
        l &= (dl !=  1); // transition up

        r |= (dr == -1);
        r &= (dr !=  1);

        m |= (dm == -1);
        m &= (dm !=  1);

        last_keypress_frame = event.key * (event.action == OUR_GLFW_PRESS);
        mwheel_y_delta = event.mwheel_y_delta;
    }

    bool ClickedRecently() {
        bool b1 = frameno - lup_fn == 0;
        bool b2 = frameno - ldo_fn < 20;

        return b1 && b2;
    }
    bool LimsLWYHLastFrame(s32 x0, s32 sz_x, s32 y0, s32 sz_y) {
        s32 x_lf = x - dx;
        s32 y_lf = y - dy;

        bool b1 = (x_lf >= x0) && (x_lf <= x0 + sz_x);
        bool b2 = (y_lf >= y0) && (y_lf <= y0 + sz_y);

        return b1 && b2;
    }
    bool LimsLTWHLastFrame(s32 x0, s32 y0, s32 sz_x, s32 sz_y) {
        s32 x_lf = x - dx;
        s32 y_lf = y - dy;

        bool b1 = (x_lf >= x0) && (x_lf <= x0 + sz_x);
        bool b2 = (y_lf >= y0) && (y_lf <= y0 + sz_y);

        return b1 && b2;
    }
};
MouseTrap InitMouseTrap(int mouse_x, int mouse_y) {
    MouseTrap m;
    _memzero(&m, sizeof(MouseTrap));
    return m;
}


inline f32 PositiveSqrtMultiplier(f32 value) {
    if (value == 0) {
        value = 1;
    }
    else if (value < 0) {
        value = -1 * value;
    }
    return sqrt(value);
}


struct OrbitCamera {
    PerspectiveFrustum frustum;
    Vector3f center;
    f32 theta;
    f32 phi;
    f32 radius;
    f32 mouse2rot = 0.4f;
    f32 mouse2pan = 0.01f;
    Matrix4f view;
    Matrix4f proj;
    Matrix4f vp;

    static f32 ClampTheta(f32 theta_degs, f32 min = 0.0001f, f32 max = 180 - 0.0001f) {
        f32 clamp_up = MinF32(theta_degs, max);
        f32 result = MaxF32(clamp_up, min);
        return result;
    }
    void Update(MouseTrap m) {
        f32 sign_x = 1;

        // why
        bool invert_x = true;
        if (invert_x) {
            sign_x = - 1;
        }

        if (m.l) {
            // orbit
            theta = OrbitCamera::ClampTheta(theta - m.dy * mouse2rot);
            phi += sign_x * m.dx * mouse2rot;
        }
        else if (m.mwheel_y_delta < 0) {
            // zoom in
            f32 mult = PositiveSqrtMultiplier((f32) m.mwheel_y_delta);
            radius *= 1.1f * mult;
        }
        else if (m.mwheel_y_delta > 0) {
            // zoom out
            f32 mult = PositiveSqrtMultiplier((f32) m.mwheel_y_delta);
            radius /= 1.1f * mult;
        }
        else if (m.r) {
            // pan
            Vector3f forward = - SphericalCoordsY(theta*deg2rad, phi*deg2rad, radius);
            forward.Normalize();
            Vector3f left = y_hat.Cross(forward);
            left.Normalize();
            Vector3f right = - left;
            Vector3f up = forward.Cross(left);
            up.Normalize();
            center = center + mouse2pan * m.dx * right;
            center = center + mouse2pan * m.dy * up;
        }

        // build orbit transform
        view = TransformBuildOrbitCam(center, theta, phi, radius);
        vp = TransformBuildViewProj(view, proj);
    }
    Vector3f Forward() {
        Vector3f forward = - SphericalCoordsY(theta*deg2rad, phi*deg2rad, radius);
        return forward;
    }
    Vector3f Position() {
        Vector3f position = center + SphericalCoordsY(theta*deg2rad, phi*deg2rad, radius);
        return position;
    }
    Ray CameraRay() {
        Vector3f forward = - SphericalCoordsY(theta*deg2rad, phi*deg2rad, radius);
        Vector3f position = center + SphericalCoordsY(theta*deg2rad, phi*deg2rad, radius);
        Ray camray { position, forward };
        return camray;
    }
};
OrbitCamera InitOrbitCamera(float aspect) {
    OrbitCamera cam { PerspectiveFrustum { 90, aspect, 0.01f, 10 } };
    cam.center = Vector3f_Zero();
    cam.theta = 60;
    cam.phi = 35;
    cam.radius = 4;
    cam.view = Matrix4f_Identity();
    cam.proj = PerspectiveMatrixOpenGL(cam.frustum, false, true, false);
    return cam;
}


#endif
