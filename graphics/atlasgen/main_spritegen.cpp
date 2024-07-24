#include <math.h>
#include <assert.h>

#include "../../baselayer.h"
#include "../gtypes.h"
#include "../font.h"
#include "../sprite.h"
#include "../resource.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


List<Sprite> CreateGridSprites(MArena *a_dest, u8* data, s32 nx, s32 ny, s32 sprite_w, s32 sprite_h, s32 bitmap_w, s32 bitmap_h, u32 texture_id) {
    // config vars
    s32 sprite_w2 = sprite_w / 2; // w2 == half width
    s32 sprite_h2 = sprite_h / 2; // h2 == half height

    // auto vars
    s32 cell_w = bitmap_w / nx;
    s32 cell_h = bitmap_h / ny;
    f32 uw = 1.0f * sprite_w / bitmap_w;
    f32 vw = 1.0f * sprite_h / bitmap_h;

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
            s.w = sprite_w;
            s.h = sprite_h;
            s.u0 = u0;
            s.u1 = u1;
            s.v0 = v0;
            s.v1 = v1;

            sprites.Add(s);
        }
    }
    return sprites;
}


void ExtractAliens() {
    MContext *ctx = InitBaselayer();

    s32 bitmap_w_01;
    s32 bitmap_h_01;
    s32 bitmap_w_02;
    s32 bitmap_h_02;
    s32 channels_in_file_01;
    s32 channels_in_file_02;
    u8 *data_01 = stbi_load("aliens_01.png", &bitmap_w_01, &bitmap_h_01, &channels_in_file_01, 4);
    u8 *data_02 = stbi_load("aliens_02.png", &bitmap_w_02, &bitmap_h_02, &channels_in_file_02, 4);
    printf("%d %d %d\n", bitmap_w_01, bitmap_h_01, channels_in_file_01);
    printf("%d %d %d\n", bitmap_w_02, bitmap_h_02, channels_in_file_02);

    assert(channels_in_file_01 == channels_in_file_02);
    assert(bitmap_w_01 == bitmap_w_02);
    assert(bitmap_h_01 == bitmap_h_02);

    ImageRGBA tex_01 { bitmap_w_01, bitmap_h_01, (Color*) data_01};
    ImageRGBA tex_02 { bitmap_w_01, bitmap_h_01, (Color*) data_02};

    // organize the two bitmap textures by id
    // TODO: get rid of the double de-referencing (first the map, then the *img) to access data
    HashMap textures = InitMap(ctx->a_life, 2);
    MapPut(&textures, 1, (u64) &tex_01);
    MapPut(&textures, 2, (u64) &tex_02);

    // lay out sprites
    List<Sprite> sprites = CreateGridSprites(ctx->a_pers, data_01, 11, 6, 36, 36, bitmap_w_01, bitmap_h_01, 1);
                           CreateGridSprites(ctx->a_pers, data_01, 11, 6, 36, 36, bitmap_w_01, bitmap_h_01, 2);
    sprites.len *= 2; // merge lists

    printf("\n");

    List<u32> tex_keys = InitList<u32>(ctx->a_tmp, sprites.len);
    tex_keys.len = sprites.len;
    for (u32 i = 0; i < tex_keys.len / 2; ++i) {
        tex_keys.lst[i] = 1;
    }
    for (u32 i = tex_keys.len / 2; i < tex_keys.len; ++i) {
        tex_keys.lst[i] = 2;
    }
    SpriteMap *smap = CompileSpriteMapInline(ctx->a_pers, "aliens", "aliens_01", sprites, tex_keys, &textures);

    // manually set all black pixels to transparant
    Color black = Color { 0, 0, 0, 255 };
    for (u32 i = 0; i < smap->texture.width * smap->texture.height; ++i) {
        Color col = smap->texture.img[i];
        if ((col.r == 0) && (col.g == 0) && (col.b == 0)) {
            smap->texture.img[i].a = 0;
        }
    }

    // drop test image to file
    stbi_write_bmp("spritemap.bmp", smap->texture.width, smap->texture.height, 4, smap->texture.img);

    // save to stream
    printf("saving to stream ...\n");
    ResourceStreamHandle hdl = ResourceStreamLoadAndOpen(ctx->a_tmp, ctx->a_life, "all.res", false);
    ResourceStreamPushData(ctx->a_life, &hdl, RT_SPRITE, smap->map_name, smap->key_name, smap, smap->size_tot);
    ResourceStreamSave(&hdl, "alt.res");
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
