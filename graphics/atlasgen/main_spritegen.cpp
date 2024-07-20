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


struct Sprite {
    Vector2i size;
    Vector2f tex_0;
    Vector2f tex_1;
    u32 tex_id;
};


List<Sprite> CreateGridSprites(MArena *a_dest, u8* data, s32 bitmap_w, s32 bitmap_h, u32 texture_id) {
    // config vars
    s32 sprite_w2 = 10; // w2 == half width
    s32 sprite_h2 = 10; // h2 == half height
    s32 nx = 11;
    s32 ny = 6;

    // auto vars
    s32 cell_w = bitmap_w / nx;
    s32 cell_h = bitmap_h / ny;
    f32 uw = sprite_w2 * 2.0f / bitmap_w;
    f32 vw = sprite_h2 * 2.0f / bitmap_h;

    // return var
    List<Sprite> sprites = InitList<Sprite>(a_dest, nx * ny);

    // create sprites
    for (u32 j = 0; j < ny; ++j) {
        for (u32 i = 0; i < nx; ++i) {
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

            printf("alien %u: x: %d, y: %d, sleft: %d, stop: %d, u0: %.4f, u1: %.4f, v0: %.4f, v1: %.4f\n", j*nx + i, x, y, sleft, stop, u0, u1, v0, v1);

            Sprite s = {};
            s.tex_id = texture_id;
            s.size = Vector2i { sprite_w2 * 2, sprite_h2 * 2 };
            s.tex_0 = Vector2f { u0, v0 };
            s.tex_1 = Vector2f { u1, v1 };

            sprites.Add(s);
        }
    }

    return sprites;
}


struct SpriteMap {
    char map_name[32];
    char key_name[32];
    List<Sprite> sprites;
    ImageRGBA texture;
};
SpriteMap CompileSpriteMapInline(MArena *a_dest, List<Sprite> sprites, HashMap *texture_map) {
    SpriteMap smap;

    // TODO: impl.

    return smap;
}


void ExtractAliens() {

    MContext *ctx = InitBaselayer();

    s32 bitmap_w;
    s32 bitmap_h;
    s32 bitmap_w_02;
    s32 bitmap_h_02;
    s32 channels_in_file_01;
    s32 channels_in_file_02;
    u8 *data_01 = stbi_load("aliens_01.png", &bitmap_w, &bitmap_h, &channels_in_file_01, 3);
    printf("%d %d %d\n", bitmap_w, bitmap_h, channels_in_file_01);
    u8 *data_02 = stbi_load("aliens_02.png", &bitmap_w, &bitmap_h, &channels_in_file_02, 3);
    printf("%d %d %d\n", bitmap_w_02, bitmap_h_02, channels_in_file_02);

    assert(channels_in_file_01 == channels_in_file_02);
    assert(bitmap_w == bitmap_w_02);
    assert(bitmap_h == bitmap_h_02);

    ImageRGBA tex_01 { bitmap_w, bitmap_h, (Color*) data_01};
    ImageRGBA tex_02 { bitmap_w, bitmap_h, (Color*) data_02};

    // organize the two bitmap textures by id
    // TODO: get rid of the double de-referencing (first the map, then the *img) to access data
    HashMap textures = InitMap(ctx->a_life, 2);
    MapPut(&textures, 0, (u64) &tex_01);
    MapPut(&textures, 1, (u64) &tex_02);

    // lay out sprites
    List<Sprite> sprites = CreateGridSprites(ctx->a_pers, data_01, bitmap_w, bitmap_h, 0);
    CreateGridSprites(ctx->a_pers, data_01, bitmap_w, bitmap_h, 1);
    sprites.len *= 2; // merge lists


    SpriteMap smap = CompileSpriteMapInline(ctx->a_pers, sprites, &textures);


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

        ExtractAliens();
    }
}
