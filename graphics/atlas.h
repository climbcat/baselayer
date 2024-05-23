#ifndef __ATLAS_H__
#define __ATLAS_H__


#include "geometry.h"


struct Glyph {
    u32 c;

    // posiitioning wrt. current point
    s32 x0; // left side bearing
    s32 y0; // y ascent (most often < 0)
    s32 w; // glyph width
    s32 h; // glyph height

    // advance to next glyph
    s32 adv_x;

    // atlas texture coords
    float tx0;
    float ty0;
    float tx1;
    float ty1;
};


struct GlyphQuadVertex {
    Vector2f pos;
    Vector2f tex;
};


struct GlyphQuad {
    GlyphQuadVertex verts[6];
};


GlyphQuad GlyphQuadCook(Glyph g) {
    GlyphQuad q;

    Vector2f ulc_pos { (f32) g.x0, (f32) g.y0 };
    Vector2f ulc_tex { (f32) g.tx0, (f32) g.ty0 };
    Vector2f urc_pos { (f32) g.x0 + g.w, (f32) g.y0 };
    Vector2f urc_tex { (f32) g.tx1, (f32) g.ty0 };
    Vector2f lrc_pos { (f32) g.x0, (f32) g.y0 + g.h };
    Vector2f lrc_tex { (f32) g.tx1, (f32) g.ty1 };
    Vector2f llc_pos { (f32) g.x0 + g.w, (f32) g.y0 + g.h };
    Vector2f llc_tex { (f32) g.tx0, (f32) g.ty1 };

    q.verts[0] = GlyphQuadVertex { urc_pos, urc_tex };
    q.verts[1] = GlyphQuadVertex { lrc_pos, lrc_tex };
    q.verts[2] = GlyphQuadVertex { llc_pos, llc_tex };
    q.verts[3] = GlyphQuadVertex { llc_pos, llc_tex };
    q.verts[4] = GlyphQuadVertex { ulc_pos, ulc_tex };
    q.verts[5] = GlyphQuadVertex { urc_pos, urc_tex };

    return q;
}


inline
GlyphQuad GlyphQuadOffset(GlyphQuad *q, Vector2f os) {
    GlyphQuad out;
    for (u32 i = 0; i < 6; ++i) {
        GlyphQuadVertex v = q->verts[i];
        v.pos.x += os.x;
        v.pos.y += os.y;
        out.verts[i] = v;
    }
    return out;
}


struct FontAtlas {
    u32 sz_px;
    u32 b_width;
    u32 b_height;
    u32 cell_width;
    u32 cell_height;
    u32 max_ascend;
    List<Glyph> glyphs;
    u8 *bitmap;

    void Print() {
        printf("font_sz %u, bitmap_sz %u %u, cell_sz %u %u, glyphs %u, data ptrs %p %p\n", sz_px, b_width, b_height, cell_width, cell_height, glyphs.len, glyphs.lst, bitmap);
    }
    void PrintGlyphsQuick() {
        for (u32 i = 32; i < glyphs.len; ++i) {
            Glyph g = glyphs.lst[i];
            printf("%c ", g.c);
        }
        printf("\n");
    }
};


FontAtlas *FontAtlasLoadBinary128(MArena *a_dest, char *filename) {
    u64 sz_file;
    u8 *base_ptr = (u8*) LoadFileMMAP(filename, &sz_file);
    base_ptr = (u8*) ArenaPush(a_dest, base_ptr, sz_file); // move to read-write memory location
    FontAtlas *atlas = (FontAtlas*) base_ptr;

    u32 sz_base = sizeof(FontAtlas);
    u32 sz_glyphs = 128 * sizeof(Glyph);
    u32 sz_bitmap = atlas->b_width * atlas->b_height;

    assert(sz_file == sz_base + sz_glyphs + sz_bitmap && "sanity check loaded file size");

    // set pointers
    atlas->glyphs.lst = (Glyph*) (base_ptr + sz_base);
    atlas->bitmap = base_ptr + sz_base + sz_glyphs;

    return atlas;
};


void FontAtlasSaveBinary128(MArena *a_tmp, char *filename, FontAtlas atlas) {
    u32 sz_base = sizeof(FontAtlas);
    u32 sz_glyphs = 128 * sizeof(Glyph);
    u32 sz_bitmap = atlas.b_width * atlas.b_height;

    FontAtlas *atlas_inlined = (FontAtlas*) ArenaPush(a_tmp, &atlas, sz_base);
    ArenaPush(a_tmp, atlas_inlined->glyphs.lst, sz_glyphs);
    ArenaPush(a_tmp, atlas_inlined->bitmap, sz_bitmap);

    // invalidate pointers
    atlas_inlined->glyphs.lst = 0;
    atlas_inlined->bitmap = 0;

    SaveFile(filename, (u8*) atlas_inlined, sz_base + sz_glyphs + sz_bitmap);
}


#endif
