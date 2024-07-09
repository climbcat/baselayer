#ifndef __ATLAS_H__
#define __ATLAS_H__


#include "geometry.h"
#include "gtypes.h"
#include "sprite.h"


//
// Glyph


struct Glyph {
    u32 c; // represents ascii char

    s32 x0; // left side bearing
    s32 y0; // y ascent (most often < 0)
    s32 w; // glyph width
    s32 h; // glyph height
    s32 adv_x; // advance to next glyph

    f32 tx0; // texture coords
    f32 ty0;
    f32 tx1;
    f32 ty1;
};
QuadHexaVertex GlyphQuadCook(Glyph g) {
    QuadHexaVertex q;

    Vector2f ulc_pos { (f32) g.x0, (f32) g.y0 };
    Vector2f ulc_tex { (f32) g.tx0, (f32) g.ty0 };

    Vector2f urc_pos { (f32) g.x0 + g.w, (f32) g.y0 };
    Vector2f urc_tex { (f32) g.tx1, (f32) g.ty0 };
    
    Vector2f lrc_pos { (f32) g.x0 + g.w, (f32) g.y0 + g.h };
    Vector2f lrc_tex { (f32) g.tx1, (f32) g.ty1 };

    Vector2f llc_pos { (f32) g.x0, (f32) g.y0 + g.h };
    Vector2f llc_tex { (f32) g.tx0, (f32) g.ty1 };

    q.verts[0] = QuadVertex { urc_pos, urc_tex };
    q.verts[1] = QuadVertex { lrc_pos, lrc_tex };
    q.verts[2] = QuadVertex { llc_pos, llc_tex };
    q.verts[3] = QuadVertex { llc_pos, llc_tex };
    q.verts[4] = QuadVertex { ulc_pos, ulc_tex };
    q.verts[5] = QuadVertex { urc_pos, urc_tex };

    return q;
}


//
//  FontAtlas


struct FontAtlas {
    ImageB texture;
    u32 sz_px;
    u32 cell_width;
    List<Glyph> glyphs;
    char font_name[32];
    char key_name[32];

    // previously known as da GLYPH PLOTTA !
    s32 ln_height;
    s32 ln_measured;
    s32 ln_ascend;
    s32 ln_descend;
    List<u8> advance_x;
    List<QuadHexaVertex> cooked;

    Glyph glyphs_mem[128];
    u8 advance_x_mem[128];
    QuadHexaVertex cooked_mem[128];

    u64 GetKey() {
        return HashStringValue(key_name);
    }

    void Print() {
        printf("font_sz %u, bitmap_sz %u %u, cell_w %u, ln_height %u, ln_ascend %u, glyphs %u, data ptrs %p %p\n", sz_px, texture.width, texture.height, cell_width, ln_height, ln_ascend, glyphs.len, glyphs.lst, texture.img);
    }
    void PrintGlyphsQuick() {
        for (u32 i = 32; i < glyphs.len; ++i) {
            Glyph g = glyphs.lst[i];
            printf("%c ", g.c);
        }
        printf("\n");
    }
};
FontAtlas *FontAtlasLoadBinaryStream(u8 *base_ptr, u32 sz_data) {
    FontAtlas *atlas = (FontAtlas*) base_ptr;
    u32 sz_base = sizeof(FontAtlas);
    u32 sz_bitmap = atlas->texture.width * atlas->texture.height;
    assert(sz_data == sz_base + sz_bitmap && "sanity check data size");

    // set pointers
    atlas->glyphs = { atlas->glyphs_mem, 128 };
    atlas->texture.img = base_ptr + sz_base;
    atlas->advance_x = { &atlas->advance_x_mem[0], 128 };
    atlas->cooked = { &atlas->cooked_mem[0], 128 };

    return atlas;
};
FontAtlas *FontAtlasLoadBinary128(MArena *a_dest, char *filename, u32 *sz = NULL) {
    u64 sz_file;
    u8 *base_ptr = (u8*) LoadFileMMAP(filename, &sz_file);
    base_ptr = (u8*) ArenaPush(a_dest, base_ptr, sz_file); // move to read-write memory location
    if (sz != NULL) {
        *sz = (u32) sz_file;
    }

    return FontAtlasLoadBinaryStream(base_ptr, sz_file);
};
void FontAtlasSaveBinary128(MArena *a_tmp, char *filename, FontAtlas atlas) {
    u32 sz_base = sizeof(FontAtlas);
    u32 sz_bitmap = atlas.texture.width * atlas.texture.height;

    FontAtlas *atlas_inlined = (FontAtlas*) ArenaPush(a_tmp, &atlas, sz_base);
    ArenaPush(a_tmp, atlas_inlined->texture.img, sz_bitmap);

    // invalidate pointers
    atlas_inlined->glyphs.lst = 0;
    atlas_inlined->texture.img = 0;

    SaveFile(filename, (u8*) atlas_inlined, sz_base + sz_bitmap);
}


void GlyphPlotterPrint(FontAtlas *plt) {
    printf("ln_height: %d\n", plt->ln_height);
    printf("ln_ascend: %d\n", plt->ln_ascend);
    for (u32 i = 0; i - plt->advance_x.len; ++i) {
        u8 adv_x = plt->advance_x.lst[i];
        printf("%d ", adv_x);
    }
    printf("tex_w: %d\n", plt->texture.width);
    printf("tex_h: %d\n", plt->texture.height);
    printf("tex_ptr: %lu\n", (u64) plt->texture.img);
    printf("\n");
}


enum FontSize {
    FS_18,
    FS_24,
    FS_30,
    FS_36,
    FS_48,
    FS_60,
    FS_72,
    FS_84,

    FS_CNT,
};
//  Notes on fonts and sizes: 
//
//  ATM, there are 8 different sizes, and only one font. 
//  When fonts are loaded dynamically, there will be 8 * nfonts entries in the pointer list, and 
//  a slightly more complicated keying. Maybe we can just add 8*font_idx to the font_size.
static HashMap g_font_map;
static FontAtlas *g_text_plotter;
FontAtlas *SetFontAndSize(FontSize font_size) {
    u32 sz_px = 0;
    switch (font_size) {
        case FS_18: sz_px = 18; break;
        case FS_24: sz_px = 24; break;
        case FS_30: sz_px = 30; break;
        case FS_36: sz_px = 36; break;
        case FS_48: sz_px = 48; break;
        case FS_60: sz_px = 60; break;
        case FS_72: sz_px = 72; break;
        case FS_84: sz_px = 84; break;
        default: break;
    }

    char buff[8];
    sprintf(buff, "%.2u", sz_px);
    Str key_name = StrCat(StrL("cmunrm_"), StrL(buff));
    u64 key = HashStringValue(StrZeroTerm(key_name));
    u64 val = MapGet(&g_font_map, key);
    g_text_plotter = (FontAtlas*) val;

    // put texture_b into the texture_b map
    return g_text_plotter;
}
FontSize GetFontSize() {
    s32 sz_px = g_text_plotter->sz_px;
    switch (sz_px) {
        case 18: return FS_18; break;
        case 24: return FS_24; break;
        case 30: return FS_30; break;
        case 36: return FS_36; break;
        case 48: return FS_48; break;
        case 60: return FS_60; break;
        case 72: return FS_72; break;
        case 84: return FS_84; break;
        default: return FS_CNT;
    }
}


#include "resource.h"


void InitFonts(MContext *ctx) {
    assert(g_texb_map.slots.len != 0 && "check sprites were initialized");
    if (g_font_map.slots.len != 0) {
        printf("WARN: re-init fonts\n");
        return;
    }

    g_font_map = InitMap(ctx->a_life, 10);

    u64 sz_file;
    u8 *readonly_data = (u8*) LoadFileMMAP("all.res", &sz_file);
    u8 *resource_data = (u8*) ArenaPush(ctx->a_life, readonly_data, sz_file);

    if (resource_data == NULL) {
        printf("please supply an 'all.res' font resource file with the executable, exiting ...\n");
        exit(0);
    }
    ResourceStreamLoad(ctx->a_life, resource_data, &g_font_map, &g_texb_map);

    // TODO: put atlas textures into the texture map, using the u64 keys

    SetFontAndSize(FS_48);
}



//
//  Text layout
//


// TODO: export helper functions to string.h


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
bool IsAscii(char c) {
    bool result = c >= 0 && c < 128;
    return result;
}
inline
Str StrInc(Str s, u32 inc) {
    assert(inc <= s.len);
    s.str += inc;
    s.len -= inc;
    return s;
}


inline
void ScaleTextInline(List<QuadHexaVertex> text, f32 scale, s32 x0, s32 y0, s32 w, s32 h) {
    if (scale != 1.0f) {
        for (u32 i = 0; i < text.len; ++i) {
            QuadHexaVertex *q = text.lst + i;

            for (u32 j = 0; j < 6; ++j) {
                Vector2f *pos = &(q->verts + j)->pos;
                pos->x = x0 + (pos->x - x0) * scale;
                pos->y = y0 + (pos->y - y0) * scale;
            }
        }
    }
}
inline
u32 WorldLen(Str s, List<u8> advance_x, s32 *w_adv) {
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
    if (pt_y + ln_ascend + 2 * ln_height < ln_limit_y) {
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
inline
void DoWhiteSpace(s32 space_width, s32 *pt_x) {
    *pt_x += space_width;
}


enum TextAlign {
    TAL_LEFT,
    TAL_CENTER,
    TAL_RIGHT,

    TAL_CNT,
};
inline
void AlignQuadsH(List<QuadHexaVertex> line_quads, s32 cx, TextAlign ta) {
    if (ta != TAL_LEFT && line_quads.len > 0) {
        QuadHexaVertex *ql = line_quads.lst;
        QuadHexaVertex *qr = line_quads.LastPtr();
        s32 line_c = (qr->GetX1() + ql->GetX0()) / 2;

        s32 offset_x = cx - line_c;
        if (ta == TAL_RIGHT) {
            offset_x *= 2;
        }
        else {
            assert(ta == TAL_CENTER);
        }

        for (u32 i = 0; i < line_quads.len; ++i) {
            QuadOffset(line_quads.lst + i, offset_x, 0);
        }
    }
}


//
// TODO: un-retire autowrap function at some point !

List<QuadHexaVertex> LayoutTextAutowrap(MArena *a_dest, FontAtlas *plt, Str txt, s32 x0, s32 y0, s32 w, s32 h, Color color, TextAlign ta) {
    assert(g_text_plotter != NULL && "init text plotters first");


    s32 pt_x = x0;
    s32 pt_y = y0;
    s32 box_r = x0 + w;
    s32 box_b = y0 + h;
    s32 w_space = plt->advance_x.lst[' '];

    u32 i = 0;
    Str s = txt;

    List<QuadHexaVertex> quads = InitList<QuadHexaVertex>(a_dest, 0);
    u32 line_first_idx = 0;
    u32 line_len = 0;
    while (s.len > 0) {
        // while words

        if (IsNewLine(s)) {
            if (CanDoNewline(pt_y, plt->ln_height, plt->ln_ascend, box_b)) {
                DoNewLine(plt->ln_height, x0, &pt_x, &pt_y);

                AlignQuadsH(List<QuadHexaVertex> { quads.lst + line_first_idx, line_len }, x0 + w / 2, ta);
                line_len = 0;
                line_first_idx = quads.len;
            }
            s = StrInc(s, 1);
        }
        if (IsWhiteSpace(s)) {
            if (CanDoWhiteSpace(pt_x, w_space, box_r)) {
                DoWhiteSpace(w_space, &pt_x);
            }
            s = StrInc(s, 1);
        }

        // lookahead word len (include leading whitespace)
        s32 w_adv = 0;
        u32 w_len = WorldLen(s, plt->advance_x, &w_adv);

        // word wrap
        if (pt_x + w_adv > box_r) {
            if (CanDoNewline(pt_y, plt->ln_height, plt->ln_ascend, box_b)) {
                DoNewLine(plt->ln_height, x0, &pt_x, &pt_y);
            
                AlignQuadsH(List<QuadHexaVertex> { quads.lst + line_first_idx, line_len }, x0 + w / 2, ta);
                line_len = 0;
                line_first_idx = quads.len;
            }
            else {
                // ran out of space, exit
                break;
            }
        }

        // lay out word
        for (u32 j = 0; j < w_len; ++j) {
            char c = s.str[j];
            QuadHexaVertex q = QuadOffset(plt->cooked.lst + c, pt_x, pt_y, color);
            pt_x += plt->advance_x.lst[c];
            ArenaAlloc(a_dest, sizeof(QuadHexaVertex));
            quads.Add(q);

            line_len++;
        }
        s = StrInc(s, w_len);

        // dbg count
        ++i;
    }
    assert(quads.len <= txt.len); // quad len equals char count minus whitespaces

    // align the last line of the batch
    AlignQuadsH(List<QuadHexaVertex> { quads.lst + line_first_idx, line_len }, x0 + w / 2, ta);

    // only scale if absolutely necessary
    f32 scale = 1.0f;
    if (scale != 1) {
        ScaleTextInline(quads, scale, x0, y0, w, h);
    }

    DrawCall dc = {};
    dc.texture_b_key = plt->GetKey();
    dc.quads = quads;
    SR_Push(dc);

    return quads;
}


s32 GetLineCenterVOffset() {
    s32 result = g_text_plotter->ln_measured / 2 - g_text_plotter->ln_descend;
    return result;
}


s32 TextLineWidth(FontAtlas *plt, Str txt) {
    s32 pt_x = 0;
    s32 w_space = plt->advance_x.lst[' '];

    for (u32 i = 0; i < txt.len; ++i) {
        // while words
        char c = txt.str[i];

        if (c == ' ') {
            pt_x += w_space;
            continue;
        }
        if (IsAscii(c) == false) {
            continue;
        }

        pt_x += plt->advance_x.lst[c];
    }

    return pt_x;
}


List<QuadHexaVertex> LayoutTextLine(MArena *a_dest, FontAtlas *plt, Str txt, s32 x0, s32 y0, s32 *sz_x, Color color) {
    assert(g_text_plotter != NULL && "init text plotters first");

    s32 pt_x = x0;
    s32 pt_y = y0 - plt->ln_ascend;
    s32 w_space = plt->advance_x.lst[' '];

    List<QuadHexaVertex> quads = InitList<QuadHexaVertex>(a_dest, 0);
    for (u32 i = 0; i < txt.len; ++i) {
        // while words
        char c = txt.str[i];

        if (c == ' ') {
            pt_x += w_space;
            continue;
        }
        if (IsAscii(c) == false) {
            continue;
        }

        QuadHexaVertex q = QuadOffset(plt->cooked.lst + c, pt_x, pt_y, color);
        pt_x += plt->advance_x.lst[c];
        ArenaAlloc(a_dest, sizeof(QuadHexaVertex));
        quads.Add(q);
    }
    *sz_x = pt_x - x0;


    // TODO: update this hack to be more organized -> e.g. put assembling the drawcall outside of
    //      this function somehow, maybe in the UI_xxx calls.
    DrawCall dc = {};
    dc.texture_b_key = plt->GetKey();
    dc.quads = quads;
    SR_Push(dc);

    return quads;
}
List<QuadHexaVertex> LayoutTextLine(const char *txt, s32 x0, s32 y0, Color color) {
    s32 sz_x;
    Str txts = { (char *) txt, _strlen((char*) txt) };

    return LayoutTextLine(g_a_quadbuffer, g_text_plotter, txts, x0, y0, &sz_x, color);
}
List<QuadHexaVertex> LayoutTextLine(Str txt, s32 x0, s32 y0, s32 *sz_x, s32 *sz_y, Color color) {
    *sz_y = g_text_plotter->ln_measured;
    return LayoutTextLine(g_a_quadbuffer, g_text_plotter, txt, x0, y0, sz_x, color);
}




#endif
