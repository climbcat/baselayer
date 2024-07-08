#include <math.h>
#include <assert.h>

#include "../../baselayer.h"
#include "../gtypes.h"
#include "../atlas.h"
#include "../resource.h"


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
    //List<Glyph> glyphs = InitList<Glyph>(a_dest, sizeof(Glyph) * ascii_range);
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
        max_ascent = MinS32(y0, max_ascent);
        max_descent = MaxS32(y1, max_descent);

        if (c >= 32) {
            printf("%c: adv, lsb, x0, y0, x1, y1: %d %d %d %d %d %d\n", c, advance_x, lsb, x0, y0, x1, y1);
        }
        ++c;
    }
    printf("\n");
    printf("ascii-wide advance, ascent, descent: %d %d %d\n", max_adv, max_ascent, max_descent);
    printf("\n");

    FontAtlas atlas;
    _memzero(&atlas, sizeof(FontAtlas));
    atlas.sz_px = line_height;
    atlas.cell_width = max_adv;
    atlas.ln_height = line_height;
    atlas.ln_ascend = max_ascent;
    atlas.ln_descend = max_descent;
    atlas.ln_measured = - max_ascent + max_descent;
    atlas.texture.width = atlas.cell_width * 16;
    atlas.texture.height = atlas.ln_height * 6;
    atlas.texture.img = (u8*) ArenaAlloc(a_dest, atlas.texture.width * atlas.texture.height * sizeof(u8), true);
    atlas.glyphs = glyphs;

    f32 tex_scale_x = 1.0f / atlas.texture.width;
    f32 tex_scale_y = 1.0f / atlas.texture.height;
    u32 aidx = 0;
    for (u32 ascii = ascii_offset; ascii < ascii_range; ++ascii) {
        Glyph *g = glyphs.lst + ascii;

        s32 x = (aidx % 16) * atlas.cell_width;
        s32 y = (aidx / 16) * atlas.ln_height + atlas.ln_height - max_descent;

        x += g->x0;
        y += g->y0;

        s32 offset = y * atlas.texture.width + x;
        stbtt_MakeCodepointBitmap(&info, atlas.texture.img + offset, g->w, g->h, atlas.texture.width, scale, scale, g->c);

        // record texture coords
        g->tx0 = (f32) x * tex_scale_x;
        g->ty0 = (f32) y * tex_scale_y;
        g->tx1 = (f32) (x + g->w) * tex_scale_x;
        g->ty1 = (f32) (y + g->h) * tex_scale_y;
        printf("%c: tex coords %.2f %.2f %.2f %.2f\n",g->c, g->tx0, g->ty0, g->tx1, g->ty1);

        ++aidx;
    }
    stbi_write_png("atlas.png", atlas.texture.width, atlas.texture.height, 1, atlas.texture.img, atlas.texture.width);
    printf("\n");
    printf("wrote atlas image to atlas.png\n");
    atlas.Print();

    return atlas;
}


Str StrSprintf(Str dest, const char *s) {
    sprintf(dest.str + dest.len, "%s", s);
    dest.len += _strlen( (char*) s);
    return dest;
}
Str StrSprintf(Str dest, Str small) {
    sprintf(dest.str + dest.len, "%s", StrZeroTerm(small));
    dest.len += small.len;
    return dest;
}


int main (int argc, char **argv) {
    TimeProgram;

    bool do_test = true;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("usage: supply .ttf font file as the first arg\n");
        printf("\n");
        printf("--help:          display help (this text)\n");
        printf("--size [size]    generated font size / line height in pixels\n");
    }
    else if (argc < 2 && !do_test) {
        printf("Input a true type font to generate atlas\n");
    }
    else if (CLAContainsArg("--size", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("TODO: impl. --size\n");
    }
    
    
    const char *font_filename = "fonts/cmunrm.ttf";

    InitBaselayer();
    Str name = StrBasename( (char*) font_filename);
    StrPrint("", name, "\n");
    //char *font_filename = CLAGetFirstArg(argc, argv);

    MContext *ctx = InitBaselayer();
    u64 sz;
    u8* font = LoadFileMMAP(font_filename, &sz);
    if (font == NULL) {
        exit(0);
    }

    // data stream linked list & save file handle
    ResourceStreamHandle stream = {};

    s32 line_sizes[8] = { 18, 24, 30, 36, 48, 60, 72, 84 };
    for (u32 i = 0; i < 8; ++i) {
        s32 sz_px = line_sizes[i];

        FontAtlas atlas = CreateCharAtlas(ctx->a_pers, font, sz_px);

        // mini test
        printf("\n");
        printf("atlas to save:\n");
        for (u32 i = 32; i < atlas.glyphs.len; ++i) {
            Glyph g = atlas.glyphs.lst[i];
            printf("%c ", g.c);
        }
        printf("\n");
        printf("\n");

        Str ext = StrLiteral("atlas");
        ext = StrCat(".", ext);

        char buff[200];
        sprintf(buff, "_%d", sz_px);
        Str fname = StrCat(name, buff);
        fname = StrCat(fname, ext);
        StrPrint("", fname, "\n");

        // save/load/print again (should not appear jumbled on screen)
        FontAtlasSaveBinary128(ctx->a_tmp, StrZeroTerm(fname), atlas);

        u32 loaded_size;
        FontAtlas *loaded = FontAtlasLoadBinary128(ctx->a_pers, StrZeroTerm(fname), &loaded_size);
        printf("atlas test loading from disk (glyphs chars 32-%u):\n", loaded->glyphs.len);
        for (u32 i = 32; i < loaded->glyphs.len; ++i) {
            Glyph g = loaded->glyphs.lst[i];
            printf("%c ", g.c);
        }
        printf("\n");


        // TODO: inlined texture -> can we be suar

        // push to stream
        ResourceStreamPushData(ctx->a_life, &stream, RT_FONT, loaded->GetKeyName(), &atlas, sizeof(atlas));
    }
    ResourceStreamSave(&stream);
}
