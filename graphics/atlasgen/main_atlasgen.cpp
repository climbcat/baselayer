#include "../../baselayer.h"
#include "../atlas.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"


FontAtlas CreateCharAtlas(MArena *a_dest, u8 *font, s32 line_height) {
    // prepare font
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, font, 0)) {
        printf("failed\n");
    }

    // calculate font scaling
    f32 scale = stbtt_ScaleForPixelHeight(&info, line_height);
    u32 ascii_range = 128;
    u32 ascii_offset = 32;
    u32 nchars = ascii_range - ascii_offset;
    List<Glyph> glyphs = InitList<Glyph>(a_dest, sizeof(Glyph) * ascii_range);

    printf("\n");
    printf("line height, scale: %d %f\n", line_height, scale);
    printf("\n");

    s32 max_adv = 0;
    s32 max_ascent = 0;
    s32 max_descent = 0;

    char c = 0;
    //while (c >= ascii_offset && c < ascii_range) {
    while (c < ascii_range) {
        // glyph metrics
        s32 x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&info, c, scale, scale, &x0, &y0, &x1, &y1);

        s32 advance_x;
    	s32 lsb;
        stbtt_GetCodepointHMetrics(&info, c, &advance_x, &lsb);
        advance_x = roundf(scale * advance_x);
        lsb = roundf(scale * lsb);

        // put metrics into glyph struct
        Glyph gl;
        gl.c = c;
        gl.x0 = lsb;
        gl.y0 = y0;
        gl.w = x1 - x0;
        gl.h = y1 - y0;
        gl.adv_x = advance_x;
        glyphs.Add(gl);

        max_adv = MaxS32(advance_x, max_adv);
        max_ascent = MaxS32(-y0, max_ascent);
        max_descent = MaxS32(y1, max_descent);

        printf("%c: adv, lsb, x0, y0, x1, y1: %d %d %d %d %d %d\n", c, advance_x, lsb, x0, y0, x1, y1);
        ++c;
    }
    printf("\n");
    printf("ascii-wide advance, ascent, descent: %d %d %d\n", max_adv, max_ascent, max_descent);
    printf("\n");

    FontAtlas atlas;
    atlas.sz_px = line_height;
    atlas.cell_width = max_adv;
    atlas.cell_height = max_ascent + max_descent;
    atlas.b_width = atlas.cell_width * 16;
    atlas.b_height = atlas.cell_height * 6;
    atlas.bitmap = (u8*) ArenaAlloc(a_dest, atlas.b_width * atlas.b_height * sizeof(u8), true);
    atlas.glyphs = glyphs;

    f32 tex_scale_x = 1.0f / atlas.b_width;
    f32 tex_scale_y = 1.0f / atlas.b_height;
    u32 aidx = 0;
    for (u32 ascii = ascii_offset; ascii < ascii_range; ++ascii) {
        Glyph *g = glyphs.lst + ascii;

        s32 x = (aidx % 16) * atlas.cell_width;
        s32 y = (aidx / 16) * atlas.cell_height + atlas.cell_height - max_descent;

        x += g->x0;
        y += g->y0;

        s32 offset = y * atlas.b_width + x;
        stbtt_MakeCodepointBitmap(&info, atlas.bitmap + offset, g->w, g->h, atlas.b_width, scale, scale, g->c);

        // record texture coords
        g->tx0 = (f32) x * tex_scale_x;
        g->ty0 = (f32) y * tex_scale_y;
        g->tx1 = (f32) (x + g->w) * tex_scale_x;
        g->ty1 = (f32) (y + g->h) * tex_scale_y;
        printf("%c: tex coords %.2f %.2f %.2f %.2f\n",g->c, g->tx0, g->ty0, g->tx1, g->ty1);

        ++aidx;
    }
    stbi_write_png("atlas.png", atlas.b_width, atlas.b_height, 1, atlas.bitmap, atlas.b_width);
    printf("\n");
    printf("wrote atlas image to atlas.png\n");
    atlas.Print();

    // TODO: write a test image e.g. 
    const char *word = "the quick brown fox";
    // ...

    return atlas;
}


int main (int argc, char **argv) {
    TimeProgram;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("usage: supply font as the first arg\n");
        printf("\n");
        printf("--help:          display help (this text)\n");
        exit(0);
    }
    if (argc != 2) {
        printf("Input a true type font to generate atlas\n");
        exit(0);
    }
    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
        exit(0);
    }
    
    //const char *font_filename = "font/cmunrm.ttf";
    char *font_filename = CLAGetFirstArg(argc, argv);

    MContext *ctx = InitBaselayer();
    u64 sz;
    u8* font = LoadFileMMAP(font_filename, &sz);
    if (font == NULL) {
        exit(0);
    }

    FontAtlas atlas = CreateCharAtlas(ctx->a_pers, font, 64);
    FontAtlasSaveBinary(ctx->a_tmp, (char*) "output.atlas", &atlas);
}
