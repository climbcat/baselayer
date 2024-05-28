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
inline
GlyphQuad GlyphQuadOffset(GlyphQuad *q, s16 x, s16 y) {
    GlyphQuad out;
    for (u32 i = 0; i < 6; ++i) {
        GlyphQuadVertex v = *(q->verts + i);
        v.pos.x += x;
        v.pos.y += y;
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
//
//  GlyphPlotter:   Source for laying out text using cooked quads - Atlas input for initialization.
//  TextBox:        [box: lrwh] Universal text-containing rectangle including all universally possible configurations.
//


struct GlyphPlotter {
    s32 ln_height;
    s32 ln_ascend;
    // TODO: color
    List<u8> advance_x;
    List<GlyphQuad> cooked;
    ImageB texture;
};
GlyphPlotter *InitGlyphPlotter(MArena *a_dest, List<Glyph> glyphs, FontAtlas *atlas) {
    GlyphPlotter *plt = (GlyphPlotter *) ArenaAlloc(a_dest, sizeof(GlyphPlotter));
    plt->advance_x = InitList<u8>(a_dest, 128);
    plt->cooked = InitList<GlyphQuad>(a_dest, 128 * sizeof(GlyphQuad));
    plt->texture = ImageB { atlas->b_width, atlas->b_height, atlas->bitmap };
    plt->ln_height = atlas->cell_height;
    plt->ln_ascend = atlas->max_ascend;

    for (u32 i = 0; i < 128; ++i) {
        Glyph g = glyphs.lst[i];
        GlyphQuad q = GlyphQuadCook(g);
        plt->cooked.lst[i] = q;
        plt->advance_x.lst[i] = g.adv_x;
    }

    return plt;
}


struct TextBox {
    s16 l;
    s16 t;
    s16 w;
    s16 h;
};
TextBox InitTextBox(s16 left, s16 top, s16 width, s16 height) {
    TextBox box { left, top, width, height };
    return box;
}


inline
bool IsWhiteSpace(char c) {
    bool result = (c == ' ' || c == '\n' || c == '\t');
    return result;
}
inline
bool IsWhiteSpace(Str s) {
    assert(s.len > 0);
    return IsWhiteSpace(s.str[0]);
}
inline
bool IsNewLine(char c) {
    bool result = (c == '\n');
    return result;
}
inline
bool IsNewLine(Str s) {
    assert(s.len > 0);
    return IsNewLine(s.str[0]);
}
inline
Str StrInc(Str s, u32 inc) {
    assert(inc <= s.len);
    s.str += inc;
    s.len -= inc;
    return s;
}


void ScaleTextInline(List<GlyphQuad> text, f32 scale, TextBox *box) {
    if (scale != 1.0f) {
        for (u32 i = 0; i < text.len; ++i) {
            GlyphQuad *q = text.lst + i;

            for (u32 j = 0; j < 6; ++j) {
                Vector2f *pos = &(q->verts + j)->pos;
                pos->x = box->l + (pos->x - box->l) * scale;
                pos->y = box->t + (pos->y - box->t) * scale;
            }
        }
    }
}
inline
u32 WorldLen(Str s, List<u8> advance_x, u32 *w_adv) {
    u32 i = 0;
    *w_adv = 0;
    while (i < s.len && IsWhiteSpace(s.str[i]) == false) {
        *w_adv += advance_x.lst[s.str[i]];
        ++i;
    }
    return i;
}
inline
bool CanDoNewline(s32 pt_y, s32 ln_height, s32 ln_ascend, s32 ln_limit_y) {
    bool result = false;
    if (pt_y - ln_ascend + 2 * ln_height < ln_limit_y) {
        result = true;
    }
    return result;
}
inline
bool CanDoWhiteSpace(s32 pt_x, s32 w_space, s32 ln_limit_x) {
    bool result = pt_x + w_space <= ln_limit_x;
    return result;
}
inline
void DoNewLine(s32 ln_height, s32 left, s32 *pt_x, s32 *pt_y) {
    *pt_y += ln_height;
    *pt_x = left;
}
void DoWhiteSpace(s32 space_width, s32 *pt_x) {
    *pt_x += space_width;
}
List<GlyphQuad> LayoutText(MArena *a_dest, Str txt, TextBox *box, GlyphPlotter *plt, f32 scale = 1.0f) {
    s32 pt_x = box->l;
    s32 pt_y = box->t + plt->ln_ascend;
    s32 box_r = box->l + box->w;
    s32 box_b = box->t + box->h;
    s32 w_space = plt->advance_x.lst[' '];

    u32 i = 0;
    Str s = txt;

    List<GlyphQuad> layed_out = InitList<GlyphQuad>(a_dest, txt.len * sizeof(GlyphQuad));
    while (s.len > 0) {
        // while words

        if (IsNewLine(s) && CanDoNewline(pt_y, plt->ln_height, plt->ln_ascend, box_b)) {
            DoNewLine(plt->ln_height, box->l, &pt_x, &pt_y);
            s = StrInc(s, 1);
        }
        if (IsWhiteSpace(s) && CanDoWhiteSpace(pt_x, w_space, box_r)) {
            DoWhiteSpace(w_space, &pt_x);
            s = StrInc(s, 1);
        }

        // lookahead word len (including any leading whitespace)
        u32 w_adv;
        u32 w_len = WorldLen(s, plt->advance_x, &w_adv);

        // word wrap
        if (pt_x + w_adv > box_r) {
            if (CanDoNewline(pt_y, plt->ln_height, plt->ln_ascend, box_b)) {
                DoNewLine(plt->ln_height, box->l, &pt_x, &pt_y);
            }
            else {
                // ran out of space, exit
                return layed_out;
            }
        }

        // lay out word
        for (u32 j = 0; j < w_len; ++j) {
            char c = s.str[j];
            GlyphQuad q = GlyphQuadOffset(plt->cooked.lst + c, pt_x, pt_y);
            pt_x += plt->advance_x.lst[c];
            layed_out.Add(q);
        }
        s = StrInc(s, w_len);
    }
    assert(layed_out.len <= txt.len);


    ScaleTextInline(layed_out, scale, box);


    return layed_out;
}


// TODO: just have a tex_stride parameter, rather than using the technically superfluous ImageB type
void BlitText(List<GlyphQuad> text, ImageRGBA img, ImageB texture) {
    for (u32 i = 0; i < text.len; ++i) {
        GlyphQuad *q = text.lst + i;

        TextureCoord coords = QuadToTextureCoord(q);
        BlitRect into = QuadToBlitRect(q);

        BlitTextureU8(img, into, texture, coords);
    }
}


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
