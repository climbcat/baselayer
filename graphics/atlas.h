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
        printf("atlas: font_sz %u, bitmap_sz %u %u, cell_sz %u %u, glyphs %u, data ptrs %p %p\n", sz_px, b_width, b_height, cell_width, cell_height, glyphs.len, glyphs.lst, bitmap);
    }
};

struct FontAtlasBinaryHdr {
    u32 glyphs_ptr;
    u32 bitmap_ptr;
    FontAtlas atlas;
};

FontAtlas FontAtlasLoadBinary(char *filename) {
    FontAtlas atlas;

    u64 sz;
    FontAtlasBinaryHdr *hdr = (FontAtlasBinaryHdr*) LoadFileMMAP(filename, &sz);
    u8 *base_ptr = (u8*) hdr;

    atlas = hdr->atlas;
    atlas.glyphs.lst = (Glyph*) (base_ptr + hdr->glyphs_ptr);
    atlas.bitmap = base_ptr + hdr->bitmap_ptr;

    return atlas;
};


void FontAtlasSaveBinary(MArena *a_tmp, char *filename, FontAtlas *atlas) {
    FontAtlasBinaryHdr hdr;
    hdr.glyphs_ptr = 0;
    hdr.bitmap_ptr = 0;
    hdr.atlas = *atlas;
    hdr.atlas.glyphs.lst = 0;

    u32 sz_hdr = sizeof(hdr);
    u8 *dest = (u8*) ArenaPush(a_tmp, &hdr, sz_hdr);
    hdr.glyphs_ptr = sz_hdr;

    u32 sz_glyphs = sizeof(Glyph) * hdr.atlas.glyphs.len;
    ArenaPush(a_tmp, atlas->glyphs.lst, sz_glyphs);

    hdr.bitmap_ptr = sz_hdr + sz_glyphs;
    u32 sz_bitmap = hdr.atlas.b_height * hdr.atlas.b_width;
    ArenaPush(a_tmp, atlas->bitmap, sz_bitmap);

    SaveFile(filename, dest, sz_hdr + sz_glyphs + sz_bitmap);
}


#endif
