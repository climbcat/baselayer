#ifndef __UI_H__
#define __UI_H__


#define DEFAULT_WINDOW_WIDTH 1440
#define DEFAULT_WINDOW_HEIGHT 800


struct MouseTrap {
    s32 x = 0;
    s32 y = 0;
    s32 dx = 0;
    s32 dy = 0;
    bool held;
    bool rheld;
    bool wup;
    bool wdown;
    float wyoffset;

    bool key_space;
    bool key_s;
    bool key_esc;

    void UpdatePosition(s32 xpos, s32 ypos) {
        dx = xpos - x;
        dy = ypos - y;
        x = xpos;
        y = ypos;
    }
    void ResetKeyAndScrollFlags() {
        key_space = false;
        key_s = false;
        key_esc = false;
        wup = false;
        wdown = false;
        wyoffset = 0;
    }
    void ResetMouse(int mouse_x, int mouse_y) {
        x = mouse_x;
        y = mouse_y;
        held = false;
        rheld = false;    }
    void PrintState() {
        printf("mouse trap state: %d %d %d %d %d %d %d %d %d %d %d %f\n",
            held, rheld, wup, wdown,
            key_space, key_s, key_esc,
            x, y, dx, dy,
            wyoffset
        );
    }
};
MouseTrap InitMouseTrap(int mouse_x, int mouse_y) {
    MouseTrap m;
    m.ResetKeyAndScrollFlags();
    m.ResetMouse(mouse_x, mouse_y);
    return m;
}


inline float PositiveSqrtMultiplier(float value) {
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
    float theta;
    float phi;
    float radius;
    float mouse2rot = 0.4;
    float mouse2pan = 0.01;
    Matrix4f view;
    Matrix4f proj;
    Matrix4f vp;

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
            float mult = PositiveSqrtMultiplier(m->wyoffset);
            radius *= 1.1 * mult;
        }
        else if (m->wup) {
            float mult = PositiveSqrtMultiplier(m->wyoffset);
            radius /= 1.1 * mult;
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
    OrbitCamera cam { PerspectiveFrustum { 90, aspect, 0.01, 10 } };
    cam.center = Vector3f_Zero();
    cam.theta = 60;
    cam.phi = 35;
    cam.radius = 8;
    cam.view = Matrix4f_Identity();
    cam.proj = PerspectiveMatrixOpenGL(cam.frustum, false, false, false);
    return cam;
}


#endif
