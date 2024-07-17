#include <math.h>
#include <assert.h>

#include "../../baselayer.h"
#include "../gtypes.h"
#include "../atlas.h"
#include "../resource.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


void ExtractAliens() {

    MContext *ctx = InitBaselayer();

    s32 bitmap_w;
    s32 bitmap_h;
    s32 channels_in_file;
    u8 *data_01 = stbi_load("aliens_01.png", &bitmap_w, &bitmap_h, &channels_in_file, 3);
    printf("%d %d %d\n", bitmap_w, bitmap_h, channels_in_file);

    u8 *data_02 = stbi_load("aliens_02.png", &bitmap_w, &bitmap_h, &channels_in_file, 3);
    printf("%d %d %d\n", bitmap_w, bitmap_h, channels_in_file);

    // config vars
    s32 sprite_w2 = 10; // w2 == half width
    s32 sprite_h2 = 10; // h2 == half height

    // auto vars
    s32 nx = 11;
    s32 ny = 6;
    s32 cell_w = bitmap_w / nx;
    s32 cell_h = bitmap_h / ny;
    f32 uw = sprite_w2 * 2.0f / bitmap_w;
    f32 vw = sprite_h2 * 2.0f / bitmap_h;
    for (u32 i = 0; i < nx; ++i) {
        for (u32 j = 0; j < ny; ++j) {
            // cell center x, y
            s32 x = i * cell_w + cell_w / 2;
            s32 y = j * cell_h + cell_h / 2;

            // sprite u.l.c.
            s32 sleft = x - sprite_w2;
            s32 stop = y - sprite_h2;
            f32 u0 = 1.0f * sleft / bitmap_w;
            f32 v0 = 1.0f * stop / bitmap_h;
            f32 u1 = u0 + uw;
            f32 v1 = v0 + vw;

            // TODO: printf the coords for visual inspection
        }
    }


    stbi_write_bmp("spritemap.bmp", bitmap_w, bitmap_h, 3, data_02);
}


void Test() {
    printf("Running tests ...\n");
}


int main (int argc, char **argv) {
    TimeProgram;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
    }
    else if (CLAContainsArg("--test", argc, argv)) {
        Test();
    }
    else {
        InitBaselayer();
        ExtractAliens();
    }
}
