#include <GL/glew.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <signal.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../baselayer.h"
#include "geometry.h"
#include "shaders.h"
#include "ui.h"
#include "swrender.h"
#include "test.cpp"



void RunProgram() {
    printf("Soft rendering demo ...\n");

    u32 w = 1280;
    u32 h = 800;
    float aspect = (float) w / h;
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    SDL_Window *window = InitOGL(w, h);

    // image buffer
    u8 *image_buffer = (u8*) ArenaAlloc(a, nchannels * w * h);
    ScreenQuadTextureProgram screen;
    screen.Init(image_buffer, w, h);

    // pipeline buffers

    List<Vector3f> _vertex_buffer = InitList<Vector3f>(a, 1000);
    List<Vector2_u16> _index_buffer = InitList<Vector2_u16>(a, 1000);
    List<Vector3f> _ndc_buffer = InitList<Vector3f>(a, 1000);
    List<Vector2_u16> _screen_buffer = InitList<Vector2_u16>(a, 1000);
    
    u16 nlines = 0;
    u32 nvertices = 0;
    Vector3f *vertex_buffer = _vertex_buffer.lst;
    Vector2_u16 *index_buffer = _index_buffer.lst;
    Vector3f *ndc_buffer = _ndc_buffer.lst;
    Vector2_u16 *screen_buffer_lines = _screen_buffer.lst;

    // entities
    AABox box = InitAABox({ 0, -0.7, 0.7 }, 0.3);
    CoordAxes axes = InitCoordAxes();

    // populate vertex & line buffer
    List<Vector3f> _lst = _vertex_buffer;
    List<Vector2_u16> _ilst = _index_buffer;
    AABoxGetVerticesAndIndices(box, &_lst, &_ilst);
    CoordAxesGetVerticesAndIndices(axes, &_lst, &_ilst);
    nvertices = _lst.len;
    nlines = _ilst.len;

    printf("nvertices: %d nlines: %d\n", nvertices, nlines);

    for (u32 i = 0; i < nvertices; ++i) {
        Vector3f v = vertex_buffer[i];
        printf("%u: x: %f y: %f z: %f \n", i, v.x, v.y, v.z);
    }
    for (u32 i = 0; i < nlines; ++i) {
        Vector2_u16 idx = index_buffer[i];
        printf("i1: %u i2: %u\n", idx.x, idx.y);
    }

    // camera
    OrbitCamera cam = InitOrbitCamera(aspect);

    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f view, model, mvp;



    // NOTE: noticed that upper-left-corner constitutes screen 0,0, which means we much 
    //  want to invert the y-axis back from whence it came :>



    // orbit camera params
    u64 iter = 0;
    MouseTrap mouse = InitMouseTrap();
    float mouse2theta = 0.3;
    float mouse2phi = 0.3;
    while (Running(&mouse)) {
        XSleep(10);
        ClearToZeroRGBA(image_buffer, w, h);

        // box animation
        model = box.transform * TransformBuildRotateY(0.03f * iter);

        // orbit
        cam.Update(&mouse);
        mvp = TransformBuildMVP(model, cam.view, proj);
        iter++;

        // render
        for (u32 i = 0; i < nvertices; ++i) {
            ndc_buffer[i] = TransformPerspective(mvp, vertex_buffer[i]);
        }
        u16 nlines_torender = LinesToScreen(w, h, index_buffer, nlines, ndc_buffer, screen_buffer_lines);
        for (u32 i = 0; i < nlines_torender; ++i) {
            RenderLineRGBA(image_buffer, w, h, screen_buffer_lines[2*i + 0], screen_buffer_lines[2*i + 1]);
        }
        screen.Draw(image_buffer, w, h);
        SDL_GL_SwapWindow(window);
    }
}

int main (int argc, char **argv) {
    TimeProgram;
    bool force_testing = false;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
        exit(0);
    }
    if (CLAContainsArg("--test", argc, argv) || force_testing) {
        Test();
        exit(0);
    }

    RunProgram();

    return 0;
}
