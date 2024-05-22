#ifndef __ATLAS_H__
#define __ATLAS_H__


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"


struct Glyph {
    s32 advance;
    s32 descent;
    s32 left_side_bearing;
};

struct FontAtlas {
    u8 *bitmap;
    s32 b_width;
    s32 b_height;
    s32 cell_width;
    s32 cell_height;
    List<Glyph> glyphs;
};


void CreateCharAtlas(MArena *a_dest, u8 *font, s32 line_height) {

    // prepare font
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, font, 0)) {
        printf("failed\n");
    }

    // calculate font scaling
    f32 scale = stbtt_ScaleForPixelHeight(&info, line_height);
    const char *word = "the quick brown fox";
    printf("line height, scale: %d %f\n", line_height, scale);


    s32 x = 0;
    s32 ascent;
    s32 descent;
    s32 line_gap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);
    printf("font-wide ascent, descent, line_gap: %d %d %d\n", ascent, descent, line_gap);


    // TODO: do the thing


    /*
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    stbtt_GetFontBoundingBox(&info, &x0, &y0, &x1, &y1);
    x0 = roundf(scale * x0);
    y0 = roundf(scale * y0);
    x1 = roundf(scale * x1);
    y1 = roundf(scale * y1);
    s32 fw_w = x1 - x0;
    s32 fw_h = y1 - y0;
    printf("font-wide bounding box x0, y0, x1, y1, width, heigh: %d %d %d %d   %d %d\n", x0, y0, x1, y1, fw_w, fw_h);
    */


    s32 max_cp_w = 0;
    s32 max_cp_h = 0;


    char c = 32;
    ++c; // discount space
    while (c > 32 && c <= 126) {
        s32 x0, y0, x1, y1;
        stbtt_GetCodepointBox(&info, c, &x0, &y0, &x1, &y1);
        x0 = roundf(scale * x0);
        y0 = roundf(scale * y0);
        x1 = roundf(scale * x1);
        y1 = roundf(scale * y1);
        s32 cp_w = x1 - x0;
        s32 cp_h = y1 - y0;

        max_cp_w = MaxS32(cp_w, max_cp_w);
        max_cp_h = MaxS32(cp_h, max_cp_h);

        s32 advance_x;
    	s32 left_side_bearing;
        stbtt_GetCodepointHMetrics(&info, c, &advance_x, &left_side_bearing);
        advance_x = roundf(scale*advance_x);
        left_side_bearing = roundf(scale*left_side_bearing);


        //printf("%c: advance_x, leftside_bearing, w, h: %d %d   %d %d\n", c, advance_x, left_side_bearing, cp_w, cp_h);

        ++c;
    }
    printf("ascii-wide bounding box: %d %d\n", max_cp_w, max_cp_h);
    
    
    
    
    FontAtlas atlas;
    atlas.b_width = max_cp_w * 16;
    atlas.b_height = max_cp_h * 8;
    atlas.cell_width = max_cp_w;
    atlas.cell_height = max_cp_h;
    atlas.bitmap = (u8*) ArenaAlloc(a_dest, atlas.b_width * atlas.b_height, true);

    c = 32;
    while (c >= 32 && c <= 126) {
        s32 _advance_x;
    	s32 left_side_bearing;
        stbtt_GetCodepointHMetrics(&info, c, &_advance_x, &left_side_bearing);
        left_side_bearing = roundf(scale*left_side_bearing);

        s32 ix0, iy0, ix1, iy1;
        stbtt_GetCodepointBitmapBox(&info, c, scale, scale, &ix0, &iy0, &ix1, &iy1);
        s32 w = ix1-ix0;
        s32 h = iy1-iy0;

        s32 grid_idx = c - 32;
        s32 grid_x = grid_idx % 16;
        s32 grid_y = grid_idx / 16;

        s32 x = grid_x * atlas.cell_width + ix0 + left_side_bearing;
        s32 y = (grid_y + 1) * atlas.cell_height + iy0;

        s32 offset = y * atlas.b_width + x;
        stbtt_MakeCodepointBitmap(&info, atlas.bitmap + offset, w, h, atlas.b_width, scale, scale, c);


        printf("%c: %d %d %d %d, %d %d\n", c, ix0, iy0, ix1, iy1, w, h);

        stbi_write_png("out.png", atlas.b_width, atlas.b_height, 1, atlas.bitmap, atlas.b_width);

        ++c;
    }
    
    stbi_write_png("out.png", atlas.b_width, atlas.b_height, 1, atlas.bitmap, atlas.b_width);


/*
    int i;
    for (i = 0; i < strlen(word); ++i)
    {
        // how wide is this character
        int ax;
    	int lsb;
        stbtt_GetCodepointHMetrics(&info, word[i], &ax, &lsb);
        // (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].)

        // get bounding box for character (may be offset to account for chars that dip above or below the line)
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        // compute y (different characters have different heights)
        int y = ascent + c_y1;

        // render character (stride and offset is important here)
        int byteOffset = x + roundf(lsb * scale) + (y * b_w);
        stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, word[i]);

        // advance x
        x += roundf(ax * scale);

        // add kerning
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
        x += roundf(kern * scale);
    }
    stbi_write_png("out.png", b_w, 128, 1, bitmap, 512);
*/

}


#endif
