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


inline
Color SampleTextureRGBA(ImageRGBA *tex, f32 x, f32 y) {
    s32 i = (s32) round(tex->width * x);
    s32 j = (s32) round(tex->height * y);
    u32 idx = tex->width * j + i;
    Color b = tex->img[idx];
    return b;
}
inline
Color SampleTextureRGBASafe(ImageRGBA *tex, f32 x, f32 y, Color col_default) {
    s32 i = (s32) round(tex->width * x);
    s32 j = (s32) round(tex->height * y);
    if (i < 0 || i >= tex->width || j < 0 || j >= tex->height) {
        return col_default;
    }
    u32 idx = tex->width * j + i;
    Color b = tex->img[idx];
    return b;
}
void BlitSprite(Sprite s, s32 x0, s32 y0, ImageRGBA *img_dest, ImageRGBA *img_src) {
    s32 q_w = s.w;
    s32 q_h = s.h;
    s32 q_x0 = x0;
    s32 q_y0 = y0;

    assert(img_dest->height >= q_w);
    assert(img_dest->width >= q_h);

    u32 stride_img = img_dest->width;

    f32 q_scale_x = (s.u1 - s.u0) / q_w;
    f32 q_scale_y = (s.v1 - s.v0) / q_h;
    f32 q_u0 = s.u0;
    f32 q_v0 = s.v0;

    // i,j          : target coords
    // i_img, j_img : img coords

    for (u32 j = 0; j < q_h; ++j) {
        s32 j_img = j + q_y0;
        if (j_img < 0 || j_img > img_dest->height) {
            continue;
        }

        for (u32 i = 0; i < q_w; ++i) {
            u32 i_img = q_x0 + i;
            if (i_img < 0 || i_img > img_dest->width) {
                continue;
            }
            f32 x = q_u0 + i * q_scale_x;
            f32 y = q_v0 + j * q_scale_y;

            // TODO: how do we regularize this code?
            Color color_src = SampleTextureRGBASafe(img_src, x, y, Color { 0, 0, 0, 255 });

            if (color_src.a != 0) {
                // rudimentary alpha-blending
                u32 idx = j_img * stride_img + i_img;
                Color color_background = img_dest->img[idx];

                f32 alpha = (1.0f * color_src.a) / 255;
                Color color_blended;
                color_blended.r = floor( alpha*color_src.r ) + floor( (1-alpha)*color_background.r );
                color_blended.g = floor( alpha*color_src.g ) + floor( (1-alpha)*color_background.g );
                color_blended.b = floor( alpha*color_src.b ) + floor( (1-alpha)*color_background.b );
                color_blended.a = 255;

                img_dest->img[idx] = color_blended;
            }
        }
    }
}


struct SpriteMap {
    char map_name[32];
    char key_name[32];
    List<Sprite> sprites;
    ImageRGBA texture;
};
SpriteMap *CompileSpriteMapInline(MArena *a_dest, const char *name, const char *key_name, List<Sprite> sprites, List<u32> tex_keys, HashMap *texture_map) {
    u32 nx = floor( sqrt(sprites.len) );
    u32 ny = sprites.len / nx + 1;
    assert(sprites.len <= nx * ny);
    printf("compiling sprite map;\n    name: %s\n    key_name: %s\n", name, key_name);
    printf("sprites count x: %u, y: %u \n", nx, ny);

    // calc bitmap size
    s32 bm_w = 0;
    s32 bm_h = 0;
    for (u32 j = 0; j < ny; ++j) {
        s32 row_w = 0;
        s32 row_max_h = 0;
        for (u32 i = 0; i < nx; ++i) {
            u32 idx = i + j*nx;
            if (idx < sprites.len) {
                Sprite s = sprites.lst[idx];
                row_w += s.w;
                row_max_h = MaxS32(row_max_h, s.h);
            }
        }

        bm_w = MaxS32(bm_w, row_w);
        bm_h += row_max_h;
    }
    printf("comp. bitmap size: %d %d\n", bm_w, bm_h);

    // alloc sprite map memory
    SpriteMap *smap = (SpriteMap*) ArenaAlloc(a_dest, sizeof(SpriteMap));
    sprintf(smap->map_name, "%s", name);
    sprintf(smap->key_name, "%s", key_name);
    smap->sprites.len = sprites.len;
    smap->sprites.lst = (Sprite*) ArenaAlloc(a_dest, sizeof(Sprite) * sprites.len);
    smap->texture.width = bm_w;
    smap->texture.height = bm_h;
    smap->texture.img = (Color*) ArenaAlloc(a_dest, sizeof(Color) * bm_w * bm_h);

    // copy data to sprites & texture
    s32 x = 0;
    s32 y = 0;
    for (u32 j = 0; j < ny; ++j) {
        s32 row_max_h = 0;
        for (u32 i = 0; i < nx; ++i) {
            u32 idx = i + j*nx;
            if (idx < sprites.len) {
                Sprite s = sprites.lst[idx];
                ImageRGBA *texture = (ImageRGBA*) MapGet(texture_map, tex_keys.lst[idx]);

                BlitSprite(s, x, y, &smap->texture, texture);
                x += s.w;
                row_max_h = MaxS32(row_max_h, s.h);
                smap->sprites.lst[idx] = s;
            }
        }
        x = 0;
        y += row_max_h;

        printf("row_h: %d\n", row_max_h);
    }

    return smap;
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

    stbi_write_bmp("spritemap.bmp", smap->texture.width, smap->texture.height, 4, smap->texture.img);
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
