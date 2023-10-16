#include "../baselayer.h"
#include "geometry.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


inline
bool Cull(u32 pos_x, u32 pos_y, u32 w, u32 h) {
    bool not_result = pos_x >= 0 && pos_x < w && pos_y >= 0 && pos_y < h;
    return !not_result;
}
inline
u32 Pos2Idx(u32 pos_x, u32 pos_y, u32 width) {
    u32 result = pos_x + width * pos_y;
    return result;
}


void TestPointCloudPerspectiveProj_2() {
    printf("TestPointCloudPerspectiveProj\n");

    // Working with the "ogl" proj matrix formula.
    //
    // Now, the camera is pointing in the negative z direction! 
    //
    // The clipping works in both the far and near planes:
    // to see this, collapse the pc z range and play with the new/far
    // ranges until til pc just appears/disappears in the output
    // image.
    //
    // Regarding the z-negative direction; This is not not practical:
    // If cameras view "backwards" could we find a proj matrix that 
    // invert this? Should be a "flip transform" that flips this direction
    // and have it be baked into the proj matrix. This amounts to flipping
    // some of the signs in the proj matrix formula.... (PerspectiveMatrixOpenGL)


    MArena arena = ArenaCreate();
    MArena *a = &arena;

    // define the point cloud    
    f32 min_x = 0;
    f32 min_y = 0;
    f32 min_z = 2;
    f32 range_pc = 1;
    f32 range_x = 0.5;
    f32 range_y = 0.5;
    f32 range_z = 0.01;

    u32 npoints = 128;
    Vector3f *pc = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * npoints);

    RandInit();
    Vector3f v;
    for (u32 i = 0; i < npoints; ++i) {
        v.x = Rand01() * range_x + min_x;
        v.y = Rand01() * range_y + min_y;
        v.z = Rand01() * range_z + min_z;
        pc[i] = v;
    }

    // camera projection
    PerspectiveFrustum frustum;
    frustum.fov = 90;
    frustum.aspect = 1;
    frustum.dist_near = 0.1;
    frustum.dist_far = 6;
    Matrix4f proj = PerspectiveMatrixOpenGL(frustum);

    // camera position
    Vector3f campos { 0, 0, 0 };
    f32 angle_y = 0;

    // camera MVP matrix (model -> view -> proj with model = Id right now)
    Matrix4f view = TransformBuild(Vector3f {0, 1, 0}, angle_y, campos);
    Matrix4f world2cam = TransformGetInverse(view);
    Matrix4f MVP = proj * world2cam;

    // camera sensor / pixel space
    u32 pos_x;
    u32 pos_y;
    u32 w = 400;
    u32 h = 400;
    u8 *image = (u8*) ArenaAlloc(a, w * h);

    // render points to bitmap
    for (u32 i = 0; i < npoints; ++i) {
        Vector3f org = pc[i];
        Vector3f v = TransformPerspective(MVP, org);

        if (v.z < -1 || v.z > 1) {
            continue;
        }

        pos_x = (u32) ((v.x + 1) / 2 * w);
        pos_y = (u32) ((v.y + 1) / 2 * h);

        if (!Cull(pos_x, pos_y, w, h)) {
            image[Pos2Idx(pos_x, pos_y, w)] = 255;
        }
    }

    u32 nchannels = 1;
    stbi_write_png("perspective.png", w, h, nchannels, image, 1 * w);
}

void TestRotateCamera() {
    printf("TestRotateCamera\n");
    MArena arena = ArenaCreate();
    MArena *a = &arena;

    // define the point cloud    
    f32 min_x = -0.25;
    f32 min_y = 0.5;
    f32 min_z = -0.25;
    f32 range_x = 0.5;
    f32 range_y = 0.5;
    f32 range_z = 0.5;

    u32 npoints = 128;
    Vector3f *pc = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * npoints);

    RandInit();
    Vector3f v;
    for (u32 i = 0; i < npoints; ++i) {
        v.x = Rand01() * range_x + min_x;
        v.y = Rand01() * range_y + min_y;
        v.z = Rand01() * range_z + min_z;
        pc[i] = v;
    }

    //// define the "camera" volume / box thingee --> this is not NDC === [-1,1]^2
    f32 cam_min_x = -1;
    f32 cam_min_y = -1;
    f32 cam_min_z = -1;
    f32 cam_range_x = 2;
    f32 cam_range_y = 2;
    f32 cam_range_z = 2;

    // camera projection
    PerspectiveFrustum frustum;
    frustum.fov = 90;
    frustum.aspect = 1;
    frustum.dist_near = 0.001;
    frustum.dist_far = 100;
    Matrix4f proj = PerspectiveMatrix(frustum);

        // camera sensor / pixel space
    u32 w = 400;
    u32 h = 400;

    for (int i = 0; i < 90; ++i) {
        // camera position
        Vector3f campos { 0, 0, 2 };
        f32 angle_y = i;

        // camera MVP matrix (model -> view -> proj with model = Id right now)
        Matrix4f view = TransformBuild(Vector3f {0, 1, 0}, angle_y * deg2rad, campos);
        Matrix4f MVP = proj * view;

        u8 *image = (u8*) ArenaAlloc(a, w * h);

        // render points to bitmap
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f org = pc[i];
            Vector3f v = TransformPerspective(MVP, org);

            // cull z value: 
            if (v.z < cam_min_z || v.z > cam_min_z + cam_range_z) {
                continue;
            }

            u32 pos_x = (u32) ((v.x - cam_min_x) / cam_range_x * w);
            u32 pos_y = (u32) ((v.y - cam_min_y) / cam_range_y * h);

            if (!Cull(pos_x, pos_y, w, h)) {
                image[Pos2Idx(pos_x, pos_y, w)] = 255;
            }
        }

        u32 nchannels = 1;
        char buff[200];
        sprintf(buff, "perspective_%.2f.png", angle_y);
        printf("writing: %s\n", buff);
        stbi_write_png(buff, w, h, nchannels, image, 1 * w);
    }
}

void TestPointCloudBoxProj() {
    printf("TestPointCloud\n");

    MArena arena = ArenaCreate();
    MArena *a = &arena;

    // define the point cloud    
    f32 min_x = 0.2;
    f32 min_y = 0.2;
    f32 min_z = 0.2;

    f32 range_x = 1;
    f32 range_y = 1;
    f32 range_z = 1;

    u32 npoints = 128;
    Vector3f *pc = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * npoints);
    
    RandInit();
    Vector3f v;
    for (u32 i = 0; i < npoints; ++i) {
        v.x = Rand01() * range_x + min_x;
        v.y = Rand01() * range_y + min_y;
        v.z = Rand01() * range_z + min_z;
        pc[i] = v;
    }

    // define the "camera" volume / box thingee
    u32 cam_min_x = 0;
    u32 cam_min_y = 0;
    u32 cam_min_z = 0;

    u32 cam_range_x = 1.4;
    u32 cam_range_y = 1.1;
    u32 cam_range_z = 1.7;

    // define the sensor space
    // NOTE: If OpenGL, we need to go further into normalized screen space [-1,1]x[-1,1]
    u32 w = 400;
    u32 h = 400;
    u8 *image = (u8*) ArenaAlloc(a, w * h);

    // render points
    for (u32 i = 0; i < npoints; ++i) {
        v = pc[i];

        u32 pos_x = (u32) ((v.x - cam_min_x) / cam_range_x * w);
        u32 pos_y = (u32) ((v.y - cam_min_y) / cam_range_y * h);

        if (!Cull(pos_x, pos_y, w, h)) {
            image[Pos2Idx(pos_x, pos_y, w)] = 255;
        }
    }

    u32 nchannels = 1;
    stbi_write_png("pc_xy.png", w, h, nchannels, image, 1 * w);
}

void TestRandImage() {
    printf("TestRandImage\n");

    MArena arena = ArenaCreate();
    MArena *a = &arena;
    RandInit();

    u32 w = 640;
    u32 h = 480;
    u8 *image = (u8*) ArenaAlloc(a, w * h);
    u32 idx;
    for (u32 i = 0; i < w; ++i) {
        for (u32 j = 0; j < h; ++j) {
            idx = i + j * w;
            if (idx % 2 == 0) {
                image[idx] = RandIntMax(255);
            }
        }
    }

    u32 nchannels = 1;
    stbi_write_png("written.png", w, h, nchannels, image, 1 * w);
}

void RunProgram() {
    TimeFunction;
    printf("Executing program ...\n");

    // ...
}

int main (int argc, char **argv) {
    TimeProgram;
    bool force_testing = true;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
        exit(0);
    }
    if (CLAContainsArg("--test", argc, argv) || force_testing) {

        //TestRandImage();
        //TestPointCloudBoxProj();
        //TestRotateCamera();
        TestPointCloudPerspectiveProj_2();

        exit(0);
    }

    RunProgram();
}
