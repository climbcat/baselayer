#ifndef __ATLAS_H__
#define __ATLAS_H__


#include "geometry.h"


struct Glyph {
    u32 c; // represents ascii char

    s32 x0; // left side bearing
    s32 y0; // y ascent (most often < 0)
    s32 w; // glyph width
    s32 h; // glyph height
    s32 adv_x; // advance to next glyph

    float tx0; // texture coords
    float ty0;
    float tx1;
    float ty1;
};


struct GlyphQuadVertex {  // vertex layout
    Vector2f pos;
    Vector2f tex;
};


struct GlyphQuad { // grouping of six vertices
    GlyphQuadVertex verts[6];
};
GlyphQuad GlyphQuadCook(Glyph g) {
    // lays down two three-vertex triangles: T1 = [ urc->lrc->llc ] and T2 = [ llc->ulc->urc ]
    GlyphQuad q;

    Vector2f ulc_pos { (f32) g.x0, (f32) g.y0 };
    Vector2f ulc_tex { (f32) g.tx0, (f32) g.ty0 };

    Vector2f urc_pos { (f32) g.x0 + g.w, (f32) g.y0 };
    Vector2f urc_tex { (f32) g.tx1, (f32) g.ty0 };
    
    Vector2f lrc_pos { (f32) g.x0 + g.w, (f32) g.y0 + g.h };
    Vector2f lrc_tex { (f32) g.tx1, (f32) g.ty1 };

    Vector2f llc_pos { (f32) g.x0, (f32) g.y0 + g.h };
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
        GlyphQuadVertex v = *(q->verts + i);
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


//
//  Quad stream & sw/test blitting


struct TextureCoord {
    f32 u0;
    f32 u1;
    f32 v0;
    f32 v1;
};
inline
TextureCoord InitTextureCoord(f32 u0, f32 u1, f32 v0, f32 v1) {
    return { u0, u1, v0, v1 };
}
inline
TextureCoord QuadToTextureCoord(GlyphQuad *q) {
    f32 u0 = q->verts[2].tex.x;
    f32 u1 = q->verts[0].tex.x;
    f32 v0 = q->verts[0].tex.y;
    f32 v1 = q->verts[2].tex.y;

    return { u0, u1, v0, v1 };
}
inline
u8 SampleTexture(ImageB tex, f32 x, f32 y) {
    u32 i = (s32) round(tex.width * x);
    u32 j = (s32) round(tex.height * y);
    u32 idx = tex.width * j + i;
    u8 b = tex.img[idx];
    return b;
}


struct BlitRect {
    s32 x0;
    s32 x1;
    s32 y0;
    s32 y1;

    inline s32 GetWidth() { return x1 - x0; }
    inline s32 GetHeight() { return y1 - y0; }
};
inline
BlitRect InitBlitRect(s32 x0, s32 x1, s32 y0, s32 y1) {
    assert(x0 <= x1 && y0 <= y1);
    return { x0, x1, y0, y1 };
}
inline
BlitRect InitBlitRect2(s32 left, s32 top, s32 width, s32 height) {
    return { left, left + width, top, top + height };
}
inline
BlitRect QuadToBlitRect(GlyphQuad *q) {
    s32 x0 = (s32) q->verts[2].pos.x;
    s32 x1 = (s32) q->verts[0].pos.x;
    s32 y0 = (s32) q->verts[0].pos.y;
    s32 y1 = (s32) q->verts[2].pos.y;

    return { x0, x1, y0, y1 };
}


void BlitTextureU8(ImageRGBA img, BlitRect into, ImageB byte_texture, TextureCoord coord) {
    assert(img.height >= into.GetHeight());
    assert(img.width >= into.GetWidth());

    u32 stride_img = img.width;

    f32 coord_w = coord.u1 - coord.u0;
    f32 coord_h = coord.v1 - coord.v0;
    f32 scale_x = coord_w / into.GetWidth();
    f32 scale_y = coord_h / into.GetHeight();

    // i,j          : target coords
    // i_img, j_img : img coords

    for (u32 j = 0; j < into.GetHeight(); ++j) {
        s32 j_img = j + into.y0;
        if (j_img < 0 || j_img > img.height) {
            continue;
        }

        for (u32 i = 0; i < into.GetWidth(); ++i) {
            u32 i_img = into.x0 + i;
            if (i_img < 0 || i_img > img.width) {
                continue;
            }

            f32 x = coord.u0 + i * scale_x;
            f32 y = coord.v0 + j * scale_y;

            if (u8 b = SampleTexture(byte_texture, x, y)) {
                Color c = { b, b, b, b };
                u32 idx = j_img * stride_img + i_img;
                img.img[idx] = c;
            }
        }
    }
}


//
//  Text layout


void BlitTextSequence(char *word, Vector2f txtbox, ImageRGBA img, ImageB texture, List<u8> advances, List<GlyphQuad> cooked) {
    Vector2f pt = txtbox;
    for (u32 i = 0; i < _strlen(word); ++i) {
        char c = word[i];
        GlyphQuad q = GlyphQuadOffset(cooked.lst + c, pt);
        pt.x += advances.lst[c];

        TextureCoord coords = QuadToTextureCoord(&q);
        BlitRect into = QuadToBlitRect(&q);

        BlitTextureU8(img, into, texture, coords);
    }
}


#endif
