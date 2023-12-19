#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__


#include <math.h>
#include <assert.h>
#include "../baselayer.h"


//
// Vector


struct Vector2_u16 {
    u16 x;
    u16 y;
};
struct Vector2_s16 {
    s16 x;
    s16 y;
};


struct Matrix4f {
    float m[4][4];
};
struct Vector4f {
    float x;
    float y;
    float z;
    float w;
};
struct Vector3f {
    float x;
    float y;
    float z;

    inline
    float Norm() {
        return sqrt(x*x + y*y + z*z);
    }
    inline
    void ScalarProductOn(float f) {
        x *= f;
        y *= f;
        z *= f;
    }
    inline
    void AddBy(Vector3f v) {
        x += v.x;
        y += v.y;
        z += v.z;
    }
    inline
    void SubtractBy(Vector3f v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }
    inline
    void Normalize() {
        float f = 1 / Norm();
        x *= f;
        y *= f;
        z *= f;
    }
    inline
    void Invert() {
        x *= -1;
        y *= -1;
        z *= -1;
    }
    inline
    float Dot(Vector3f v) {
        return x*v.x + y*v.y + z*v.z;
    }
    inline
    Vector3f Cross(Vector3f v) {
        return Vector3f { y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x };
    }
    inline bool IsNonZero() {
        // TODO: how should epsilon be done in general?
        float epsilon = 0.0000000001f;
        bool x_nzero = abs(x) - epsilon >= 0;
        bool y_nzero = abs(y) - epsilon >= 0;
        bool z_nzero = abs(z) - epsilon >= 0;
        bool result = x_nzero || y_nzero || z_nzero;
        return result;
    }
    inline bool IsZero() {
        return ! IsNonZero();
    }

    // static versions
    inline
    static Vector3f Zero() {
        return Vector3f { 0, 0, 0 };
    }
    inline
    // left-handed: Left, Up, Forward
    static Vector3f Left() {
        return Vector3f { 1, 0, 0 };
    }
    inline
    static Vector3f Up() {
        return Vector3f { 0, 1, 0 };
    }
    inline
    static Vector3f Forward() {
        return Vector3f { 0, 0, 1 };
    }
    inline
    static Vector3f X() {
        return Vector3f { 1, 0, 0 };
    }
    inline
    static Vector3f Y() {
        return Vector3f { 0, 1, 0 };
    }
    inline
    static Vector3f Z() {
        return Vector3f { 0, 0, 1 };
    }
    inline
    static float NormSquared(Vector3f *a) {
        return a->x*a->x + a->y*a->y + a->z*a->z;
    }
    inline
    static float Norm(Vector3f *a) {
        return sqrt(Vector3f::NormSquared(a));
    }
    inline
    static Vector3f ScalarProduct(float f, Vector3f *a) {
        return Vector3f { f*a->x, f*a->y, f*a->z };
    }
    inline
    static Vector3f Normalize(Vector3f *a) {
        float norm_inv = 1 / Vector3f::Norm(a);
        return Vector3f { norm_inv * a->x, norm_inv * a->y, norm_inv * a->z };
    }
    inline
    static Vector3f Subtract(Vector3f *a, Vector3f *b) {
        return Vector3f { a->x - b->x, a->y - b->y, a->z - b->z };
    }
    inline
    static Vector3f Add(Vector3f *a, Vector3f *b) {
        return Vector3f { a->x + b->x, a->y + b->y, a->z + b->z };
    }
    inline
    static float Dot(Vector3f *a, Vector3f *b) {
        return a->z*b->z + a->z*b->z + a->z*b->z;
    }
    inline
    static Vector3f Cross(Vector3f *a, Vector3f *b) {
        return Vector3f { a->y*b->z - a->z*b->y, a->z*b->x - a->x*b->z, a->x*b->y - a->y*b->x };
    }
};
inline
Vector3f operator+(Vector3f u, Vector3f v) {
    return Vector3f::Add(&u, &v);
}
inline
Vector3f operator-(Vector3f u, Vector3f v) {
    return Vector3f::Subtract(&u, &v);
}
inline
bool operator==(Vector3f u, Vector3f v) {
    return (u.x == v.x) && (u.y == v.y) && (u.z == v.z);
}
inline
Vector3f operator*(float f, Vector3f v) {
    return Vector3f::ScalarProduct(f, &v);
}
inline
Vector3f operator-(Vector3f v) {
    return Vector3f::ScalarProduct(-1, &v);
}
Vector3f x_hat { 1, 0, 0 };
Vector3f y_hat { 0, 1, 0 };
Vector3f z_hat { 0, 0, 1 };
Vector3f Vector3f_Zero() {
    return Vector3f { 0, 0, 0 };
}
Vector3f SphericalCoordsY(float theta, float phi, float radius) {
    Vector3f v;
    v.x = radius * sin(theta) * cos(phi);
    v.y = radius * cos(theta);
    v.z = radius * sin(theta) * sin(phi);
    return v;
}


//
// Matrix


Matrix4f Matrix4f_Zero() {
    Matrix4f m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m.m[i][j] = 0;
        }
    }
    return m;
}
Matrix4f Matrix4f_One() {
    Matrix4f m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m.m[i][j] = 1;
        }
    }
    return m;
}
Matrix4f Matrix4f_Identity() {
    Matrix4f m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) {
                m.m[i][j] = 1;
            }
            else {
                m.m[i][j] = 0;
            }
        }
    }
    return m;
}
Matrix4f Matrix4f_Diagonal(Vector4f d) {
    Matrix4f m = Matrix4f_Zero();
    m.m[0][0] = d.x;
    m.m[1][1] = d.y;
    m.m[2][2] = d.z;
    m.m[3][3] = d.w;
    return m;
}
Matrix4f Matrix4f_Transpose(Matrix4f *m) {
    Matrix4f result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = m->m[j][i];
        }
    }
    return result;
}
Matrix4f Matrix4f_Transpose(Matrix4f m) {
    Matrix4f result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = m.m[j][i];
        }
    }
    return result;
}
Matrix4f Matrix4f_Multiply(Matrix4f *a, Matrix4f *b) {
    Matrix4f result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0;
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += a->m[i][k]*b->m[k][j];
            }
        }
    }
    return result;
}
Vector4f Matrix4f_MultVector(Matrix4f *a, Vector4f *v) {
    Vector4f result;
    result.x = a->m[0][0]*v->x + a->m[0][1]*v->y + a->m[0][2]*v->z + a->m[0][3]*v->w;
    result.y = a->m[1][0]*v->x + a->m[1][1]*v->y + a->m[1][2]*v->z + a->m[1][3]*v->w;
    result.z = a->m[2][0]*v->x + a->m[2][1]*v->y + a->m[2][2]*v->z + a->m[2][3]*v->w;
    result.w = a->m[3][0]*v->x + a->m[3][1]*v->y + a->m[3][2]*v->z + a->m[3][3]*v->w;
    return result;
}
bool Matrix4f_Equals(Matrix4f *a, Matrix4f *b) {
    bool result = true;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result &= a->m[i][j] == b->m[i][j];
        }
    }
    return result;
}
inline
Matrix4f operator*(Matrix4f a, Matrix4f b) {
    return Matrix4f_Multiply(&a, &b);
}
inline
Vector4f operator*(Matrix4f m, Vector4f v) {
    return Matrix4f_MultVector(&m, &v);
}
inline
bool operator==(Matrix4f a, Matrix4f b) {
    return Matrix4f_Equals(&a, &b);
}
inline
void MatrixNf_Transpose(float *dest, float *src, u32 dims) {
    for (u32 row = 0; row < dims; ++row) {
        for (u32 col = 0; col < dims; ++col) {
            dest[row*dims + col] = src[col*dims + row];
        }
    }
}
inline
void MatrixNf_Multiply(float *dest, float *a, float *b, u32 dims) {
    for (int i = 0; i < dims; ++i) {
        for (int j = 0; j < dims; ++j) {
            u32 I = i*dims;
            dest[I + j] = 0;
            for (int k = 0; k < dims; ++k) {
                dest[I + j] += a[I + k]*b[k*dims + j];
            }
        }
    }
}
inline
void MatrixNf_MultVector(float *dest, float *a, float *v, u32 dims) {
    for (int i = 0; i < dims; ++i) {
        dest[i] = 0;
        for (int k = 0; k < dims; ++k) {
            dest[i] += a[i*dims + k]*v[k];
        }
    }
}


//
// Transform


Matrix4f TransformBuild(Vector3f axis, float angle_rads, Vector3f translate = {0, 0, 0}) {
    Matrix4f result = Matrix4f_Zero();

    float epsilon_f = 0.0000001;
    assert( abs(axis.Norm() - 1) < epsilon_f );

    // build rot from [axis, angle] - see https://en.wikipedia.org/wiki/Rotation_matrix#Axis_and_angle
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float c = cos(angle_rads);
    float s = sin(angle_rads);

    result.m[0][0] = x*x*(1 - c) + c;
    result.m[0][1] = x*y*(1 - c) - z*s;
    result.m[0][2] = x*z*(1 - c) + y*s;

    result.m[1][0] = y*x*(1 - c) + z*s;
    result.m[1][1] = y*y*(1 - c) + c;
    result.m[1][2] = y*z*(1 - c) - x*s;

    result.m[2][0] = z*x*(1 - c) - y*s;
    result.m[2][1] = z*y*(1 - c) + x*s;
    result.m[2][2] = z*z*(1 - c) + c;

    // translation
    result.m[0][3] = translate.x;
    result.m[1][3] = translate.y;
    result.m[2][3] = translate.z;
    result.m[3][3] = 1;

    return result;
}
Matrix4f TransformBuildRotateX(float angle_rads) {
    return TransformBuild(x_hat, angle_rads);
}
Matrix4f TransformBuildRotateY(float angle_rads) {
    return TransformBuild(y_hat, angle_rads);
}
Matrix4f TransformBuildRotateZ(float angle_rads) {
    return TransformBuild(z_hat, angle_rads);
}
Matrix4f TransformBuildTranslationOnly(Vector3f translate) {
    // TODO: TEST: should be equal to a matrix with identity rot, built manually
    return TransformBuild(Vector3f {1, 0, 0}, 0, translate);
}

Matrix4f TransformGetInverse(Matrix4f *a) {
    // M^{-1}: row 0-2: R^{-1}, - R^{-1} * t; row 3: 0^, 1

    // 0^, 1
    Matrix4f result = Matrix4f_Zero();
    result.m[3][3] = 1;

    // R^{-1}
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = a->m[j][i];
        }
    }

    // R^{-1} * t
    Vector3f t_inv;
    t_inv.x = result.m[0][0]*a->m[0][3] + result.m[0][1]*a->m[1][3] + result.m[0][2]*a->m[2][3];
    t_inv.y = result.m[1][0]*a->m[0][3] + result.m[1][1]*a->m[1][3] + result.m[1][2]*a->m[2][3];
    t_inv.z = result.m[2][0]*a->m[0][3] + result.m[2][1]*a->m[1][3] + result.m[2][2]*a->m[2][3];

    // *= -1 and copy
    result.m[0][3] = - t_inv.x;
    result.m[1][3] = - t_inv.y;
    result.m[2][3] = - t_inv.z;

    return result;
}
Matrix4f TransformGetInverse(Matrix4f a) {
    return TransformGetInverse(&a);
}
inline
Vector3f TransformPoint(Matrix4f *a, Vector3f *v) {
    Vector3f result;

    // rot / trans
    result.x = a->m[0][0]*v->x + a->m[0][1]*v->y + a->m[0][2]*v->z + a->m[0][3];
    result.y = a->m[1][0]*v->x + a->m[1][1]*v->y + a->m[1][2]*v->z + a->m[1][3];
    result.z = a->m[2][0]*v->x + a->m[2][1]*v->y + a->m[2][2]*v->z + a->m[2][3];

    return result;
}
inline
Vector3f TransformPoint(Matrix4f a, Vector3f v) {
    Vector3f result;

    // rot / trans
    result.x = a.m[0][0]*v.x + a.m[0][1]*v.y + a.m[0][2]*v.z + a.m[0][3];
    result.y = a.m[1][0]*v.x + a.m[1][1]*v.y + a.m[1][2]*v.z + a.m[1][3];
    result.z = a.m[2][0]*v.x + a.m[2][1]*v.y + a.m[2][2]*v.z + a.m[2][3];

    return result;
}
// TODO: how do I build a vector that does this ?
inline
Vector3f TransformInversePoint(Matrix4f *a, Vector3f *v) {
    Vector3f r;
    Vector3f tmp;

    // translate back
    tmp.x = v->x - a->m[0][3];
    tmp.y = v->y - a->m[1][3];
    tmp.z = v->z - a->m[2][3];

    // rotate back (transpose)
    r.x = a->m[0][0]*tmp.x + a->m[1][0]*tmp.y + a->m[2][0]*tmp.z;
    r.y = a->m[0][1]*tmp.x + a->m[1][1]*tmp.y + a->m[2][1]*tmp.z;
    r.z = a->m[0][2]*tmp.x + a->m[1][2]*tmp.y + a->m[2][2]*tmp.z;

    return r;
}
inline
Vector3f TransformDirection(Matrix4f *a, Vector3f *d) {
    Vector3f result;

    // rotate
    result.x = a->m[0][0]*d->x + a->m[0][1]*d->y + a->m[0][2]*d->z;
    result.y = a->m[1][0]*d->x + a->m[1][1]*d->y + a->m[1][2]*d->z;
    result.z = a->m[2][0]*d->x + a->m[2][1]*d->y + a->m[2][2]*d->z;

    return result;
}
inline
Vector3f TransformDirection(Matrix4f a, Vector3f d) {
    Vector3f result;

    // just rotate
    result.x = a.m[0][0]*d.x + a.m[0][1]*d.y + a.m[0][2]*d.z;
    result.y = a.m[1][0]*d.x + a.m[1][1]*d.y + a.m[1][2]*d.z;
    result.z = a.m[2][0]*d.x + a.m[2][1]*d.y + a.m[2][2]*d.z;

    return result;
}
inline
Vector3f TransformInverseDirection(Matrix4f *a, Vector3f *d) {
    Vector3f result;

    // rotate back
    result.x = a->m[0][0]*d->x + a->m[1][0]*d->y + a->m[2][0]*d->z;
    result.y = a->m[0][1]*d->x + a->m[1][1]*d->y + a->m[2][1]*d->z;
    result.z = a->m[0][2]*d->x + a->m[1][2]*d->y + a->m[2][2]*d->z;

    // TODO: scale back
    return result;
}
Matrix4f TransformBuildLookRotationYUp(Vector3f at, Vector3f from) {

    Vector3f forward = at - from;
    forward.Normalize();
    Vector3f left = y_hat.Cross(forward);
    left.Normalize();
    Vector3f right = - left;
    Vector3f up = forward.Cross(left);
    up.Normalize();

    Matrix4f lookrot = Matrix4f_Identity();
    lookrot.m[0][0] = right.x;
    lookrot.m[1][0] = right.y;
    lookrot.m[2][0] = right.z;
    lookrot.m[0][1] = up.x;
    lookrot.m[1][1] = up.y;
    lookrot.m[2][1] = up.z;
    lookrot.m[0][2] = forward.x;
    lookrot.m[1][2] = forward.y;
    lookrot.m[2][2] = forward.z;

    return lookrot;
}



//
// Projections


struct LensParams {
    float fL; // focal length, typically 24 - 200 [mm]
    float N; // f-number, 1.4 to 60 dimensionless []
    float c; // circle of confusion (diameter), 0.03 [mm]
    float w; // sensor width, 35.9 [mm]
    float h; // sensor height, 24 [mm]
};
struct PerspectiveFrustum {
    float fov; // [degs] (horizontal field of view)
    float aspect; // [1] (width divided by height)
    float dist_near; // [m]
    float dist_far; // [m]
};
Matrix4f Matrix4f_FlipX() {
    Matrix4f flip = Matrix4f_Identity();
    flip.m[0][0] = -1;
    return flip;
}
Matrix4f Matrix4f_FlipY() {
    Matrix4f flip = Matrix4f_Identity();
    flip.m[1][1] = -1;
    return flip;
}
Matrix4f Matrix4f_FlipZ() {
    Matrix4f flip = Matrix4f_Identity();
    flip.m[2][2] = -1;
    return flip;
}
Matrix4f PerspectiveMatrixOpenGL(PerspectiveFrustum frustum, bool flip_x = true, bool flip_y = false, bool flip_z = true) {
    // gather values
    float f = frustum.dist_far;
    float n = frustum.dist_near;
    float r = frustum.dist_near * sin(frustum.fov / 2 * deg2rad);
    float l = -r;
    float b = r / frustum.aspect;
    float t = -b;

    // populate
    Matrix4f m = Matrix4f_Zero();
    m.m[0][0] = 2 * n / (r - l);
    m.m[0][2] = (r + l) / (r - l);
    m.m[1][1] = 2 * n / (t - b);
    m.m[1][2] = (t + b) / (t - b);
    m.m[2][2] = -(f + n) / (f - n);
    m.m[2][3] = -2 * f * n / (f - n);
    m.m[3][2] = -1;

    // flip the axes (flip to suit desired axis configurations)
    Matrix4f flip = Matrix4f_Identity();
    if (flip_x) {
        flip.m[0][0] = -1;
        m = flip * m;
    }
    if (flip_y) {
        flip.m[0][0] = 1;
        flip.m[1][1] = -1;
        m = flip * m;
    }
    if (flip_z) {
        flip.m[0][0] = 1;
        flip.m[1][1] = 1;
        flip.m[2][2] = -1;
        m = flip * m;
    }

    return m;
}
inline
Matrix4f TransformBuildMVP(Matrix4f model, Matrix4f view, Matrix4f proj) {
    Matrix4f mvp = proj * TransformGetInverse( view ) * model;
    return mvp;
}
inline
Matrix4f TransformBuildMVP(Matrix4f model, Matrix4f vp) {
    Matrix4f mvp = vp * model;
    return mvp;
}
inline
Matrix4f TransformBuildViewProj(Matrix4f view, Matrix4f proj) {
    Matrix4f mvp = proj * TransformGetInverse( view );
    return mvp;
}
inline
Matrix4f TransformBuildOrbitCam(Vector3f center, float theta_degs, float phi_degs, float radius) {
    Vector3f campos = center + SphericalCoordsY(theta_degs*deg2rad, phi_degs*deg2rad, radius);
    Matrix4f view = TransformBuildTranslationOnly(campos) * TransformBuildLookRotationYUp(center, campos);
    return view;
}
inline
Vector3f TransformPerspective(Matrix4f p, Vector3f v) {
    Vector4f v_hom { v.x, v.y, v.z, 1 }; // homogeneous coordinates
    Vector4f v_clip = p * v_hom; // clipping space
    Vector3f result { v_clip.x / v_clip.w, v_clip.y / v_clip.w, v_clip.z / v_clip.w }; // ndc coordinates

    return result;
}


//
// Ray


struct Ray {
    // points: (x, y, z, 1)
    // directions: (x, y, z, 0)
    Vector3f position;
    Vector3f direction;

    inline
    static Ray Zero() {
        return Ray { Vector3f::Zero(), Vector3f::Zero() };
    }
};
Ray TransformRay(Matrix4f *a, Ray *r) {
    return Ray { TransformPoint(a, &r->position), TransformDirection(a, &r->direction) };
}


//
// Utility


void PrintTransform(Matrix4f m) {
    printf("[");
    for (int i = 0; i < 4; ++i) {
        if (i > 0) {
            printf(" ");
        }
        for (int j = 0; j < 4; ++j) {
            printf(" %f", m.m[i][j]);
        }
        if (i < 3) {
            printf("\n");
        }
    }
    printf(" ]\n\n");
}
void PopulateMatrixRandomly(Matrix4f *m) {
    RandInit();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m->m[i][j] = RandMinMaxI_f32(0, 9);
        }
    }
}


#endif
