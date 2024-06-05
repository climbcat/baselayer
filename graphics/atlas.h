#ifndef __ATLAS_H__
#define __ATLAS_H__


#include "geometry.h"
#include "gtypes.h"


//
//  Quads


struct Quad {
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
    f32 u0;
    f32 v0;
    f32 u1;
    f32 v1;
    Color c;
};
struct QuadVertex {  // vertex layout
    Vector2f pos;
    Vector2f tex;
    Color col;
};
inline
QuadVertex InitQuadVertex(Vector2f pos, Vector2f tex, Color col) {
    QuadVertex v;
    v.pos = pos;
    v.tex = tex;
    v.col = col;
    return v;
}
struct QuadHexaVertex { // renderable six-vertex quad
    QuadVertex verts[6];

    inline
    Color GetColor() {
        return verts[0].col;
    }
    inline
    void SetColor(Color color) {
        for (u32 i = 0; i < 6; ++i) {
            verts[i].col = color;
        }
    }
    inline
    s32 GetWidth() {
        s32 x0 = (s32) verts[2].pos.x;
        s32 x1 = (s32) verts[0].pos.x;
        s32 width = x1 - x0;
        return width;
    }
    inline
    s32 GetHeight() {
        s32 y0 = (s32) verts[0].pos.y;
        s32 y1 = (s32) verts[2].pos.y;
        s32 width = y1 - y0;
        return width;
    }
    inline
    s32 GetX0() {
        s32 x0 = (s32) verts[2].pos.x;
        return x0;
    }
    inline
    s32 GetY0() {
        s32 y0 = (s32) verts[0].pos.y;
        return y0;
    }
    inline
    f32 GetTextureScaleX(s32 dest_width) {
        f32 u0 = verts[2].tex.x;
        f32 u1 = verts[0].tex.x;
        f32 scale_x = (u1 - u0) / dest_width;
        return scale_x;
    }
    inline
    f32 GetTextureScaleY(s32 dest_height) {
        f32 v0 = verts[0].tex.y;
        f32 v1 = verts[2].tex.y;
        f32 scale_y = (v1 - v0) / dest_height;
        return scale_y;
    }
    inline
    f32 GetTextureU0() {
        f32 u0 = verts[2].tex.x;
        return u0;
    }
    inline
    f32 GetTextureV0() {
        f32 v0 = verts[0].tex.y;
        return v0;
    }
    f32 GetTextureWidth() {
        f32 u0 = verts[2].tex.x;
        f32 u1 = verts[0].tex.x;
        return u1 - u0;
    }
    inline
    f32 GetTextureHeight() {
        f32 v0 = verts[0].tex.y;
        f32 v1 = verts[2].tex.y;
        return v1 - v0;
    }
};
QuadHexaVertex QuadCook(Quad q) {
    // lays down two three-vertex triangles: T1 = [ urc->lrc->llc ] and T2 = [ llc->ulc->urc ]
    // ulc: upper-left corner (etc.)
    QuadHexaVertex qh;

    Vector2f ulc_pos { (f32) q.x0, (f32) q.y0 };
    Vector2f urc_pos { (f32) q.x1, (f32) q.y0 };
    Vector2f lrc_pos { (f32) q.x1, (f32) q.y1 };
    Vector2f llc_pos { (f32) q.x0, (f32) q.y1 };

    Vector2f ulc_tex { (f32) q.u0, (f32) q.v0 };
    Vector2f urc_tex { (f32) q.u1, (f32) q.v0 };
    Vector2f lrc_tex { (f32) q.u1, (f32) q.v1 };
    Vector2f llc_tex { (f32) q.u0, (f32) q.v1 };

    qh.verts[0] = InitQuadVertex( urc_pos, urc_tex, q.c );
    qh.verts[1] = InitQuadVertex( lrc_pos, lrc_tex, q.c );
    qh.verts[2] = InitQuadVertex( llc_pos, llc_tex, q.c );
    qh.verts[3] = InitQuadVertex( llc_pos, llc_tex, q.c );
    qh.verts[4] = InitQuadVertex( ulc_pos, ulc_tex, q.c );
    qh.verts[5] = InitQuadVertex( urc_pos, urc_tex, q.c );

    return qh;
}
inline
QuadHexaVertex QuadOffset(QuadHexaVertex *q, Vector2f os) {
    QuadHexaVertex out;
    for (u32 i = 0; i < 6; ++i) {
        QuadVertex v = *(q->verts + i);
        v.pos.x += os.x;
        v.pos.y += os.y;
        out.verts[i] = v;
    }
    return out;
}
inline
QuadHexaVertex QuadOffset(QuadHexaVertex *q, s16 x, s16 y, Color color) {
    QuadHexaVertex out;
    for (u32 i = 0; i < 6; ++i) {
        QuadVertex v = *(q->verts + i);
        v.pos.x += x;
        v.pos.y += y;
        v.col = color;
        out.verts[i] = v;
    }
    return out;
}


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
    u32 ln_height;
    u32 ln_ascend;
    List<Glyph> glyphs;

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
FontAtlas *FontAtlasLoadBinary128(MArena *a_dest, char *filename) {
    u64 sz_file;
    u8 *base_ptr = (u8*) LoadFileMMAP(filename, &sz_file);
    base_ptr = (u8*) ArenaPush(a_dest, base_ptr, sz_file); // move to read-write memory location
    FontAtlas *atlas = (FontAtlas*) base_ptr;

    u32 sz_base = sizeof(FontAtlas);
    u32 sz_glyphs = 128 * sizeof(Glyph);
    u32 sz_bitmap = atlas->texture.width * atlas->texture.height;

    assert(sz_file == sz_base + sz_glyphs + sz_bitmap && "sanity check loaded file size");

    // set pointers
    atlas->glyphs.lst = (Glyph*) (base_ptr + sz_base);
    atlas->texture.img = base_ptr + sz_base + sz_glyphs;

    return atlas;
};
void FontAtlasSaveBinary128(MArena *a_tmp, char *filename, FontAtlas atlas) {
    u32 sz_base = sizeof(FontAtlas);
    u32 sz_glyphs = 128 * sizeof(Glyph);
    u32 sz_bitmap = atlas.texture.width * atlas.texture.height;

    FontAtlas *atlas_inlined = (FontAtlas*) ArenaPush(a_tmp, &atlas, sz_base);
    ArenaPush(a_tmp, atlas_inlined->glyphs.lst, sz_glyphs);
    ArenaPush(a_tmp, atlas_inlined->texture.img, sz_bitmap);

    // invalidate pointers
    atlas_inlined->glyphs.lst = 0;
    atlas_inlined->texture.img = 0;

    SaveFile(filename, (u8*) atlas_inlined, sz_base + sz_glyphs + sz_bitmap);
}


struct GlyphPlotter {
    s32 sz_px;
    s32 ln_height;
    s32 ln_ascend;
    List<u8> advance_x;
    List<QuadHexaVertex> cooked;
    ImageB texture;
};
GlyphPlotter *InitGlyphPlotter(MArena *a_dest, List<Glyph> glyphs, FontAtlas *atlas) {
    GlyphPlotter *plt = (GlyphPlotter *) ArenaAlloc(a_dest, sizeof(GlyphPlotter));
    plt->advance_x = InitList<u8>(a_dest, 128);
    plt->advance_x.len = 128;
    plt->cooked = InitList<QuadHexaVertex>(a_dest, 128 * sizeof(QuadHexaVertex));
    plt->cooked.len = 128;
    plt->texture = atlas->texture;
    plt->sz_px = atlas->sz_px;
    plt->ln_height = atlas->ln_height;
    plt->ln_ascend = atlas->ln_ascend;

    for (u32 i = 0; i < 128; ++i) {
        Glyph g = glyphs.lst[i];
        QuadHexaVertex q = GlyphQuadCook(g);
        plt->cooked.lst[i] = q;
        plt->advance_x.lst[i] = g.adv_x;
    }

    return plt;
}
void GlyphPlotterPrint(GlyphPlotter *plt) {
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
static GlyphPlotter *g_text_plotter;
GlyphPlotter *SetFontAndSize(FontSize font_size /*, Font font_name*/) {
    u64 sz_px = 0;
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
    u64 val = MapGet(&g_font_map, sz_px);
    g_text_plotter = (GlyphPlotter*) val;

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
void InitFonts() {
    if (g_font_map.slots.len != 0) {
        printf("WARN: re-init fonts\n");
        return;
    }

    MContext *ctx = InitBaselayer();
    StrLst *fonts = GetFilesExt("atlas");
    u32 font_cnt = StrListLen(fonts);
    printf("loading %u (.atlas) font files\n", font_cnt);

    g_font_map = InitMap(ctx->a_life, font_cnt);
    while (fonts != NULL) {
        FontAtlas *atlas = FontAtlasLoadBinary128(ctx->a_life, StrZeroTerm( fonts->GetStr() ));
        GlyphPlotter *plt = InitGlyphPlotter(ctx->a_life, atlas->glyphs, atlas);

        MapPut(&g_font_map, atlas->sz_px, plt);
        fonts = fonts->next;
    }
    SetFontAndSize(FS_48);
}


//
//  Blitting / Sprite Rendering
//
//  DrawCall


struct DrawCall {
    u32 texture;
    List<QuadHexaVertex> quads;
};
ImageB *DC_GetTexture(u32 id) {
    assert(g_text_plotter != NULL && "init a glyph plotter first");
    return &g_text_plotter->texture;
}
DrawCall InitDrawCall(List<QuadHexaVertex> quads, u32 texture) {
    DrawCall dc;
    dc.quads = quads;
    dc.texture = texture;
    return dc;
}
DrawCall InitDrawCallEmpty() {
    static DrawCall empty;
    return empty;
}


inline
u8 SampleTexture(ImageB *tex, f32 x, f32 y) {
    u32 i = (s32) round(tex->width * x);
    u32 j = (s32) round(tex->height * y);
    u32 idx = tex->width * j + i;
    u8 b = tex->img[idx];
    return b;
}
void BlitQuads(DrawCall call, ImageRGBA *img) {
    ImageB *texture = DC_GetTexture(call.texture);

    for (u32 i = 0; i < call.quads.len; ++i) {
        QuadHexaVertex *q = call.quads.lst + i;
        bool has_tex_coords = q->GetTextureWidth() > 0;

        s32 q_w = q->GetWidth();
        s32 q_h = q->GetHeight();
        s32 q_x0 = q->GetX0();
        s32 q_y0 = q->GetY0();
        assert(img->height >= q_w);
        assert(img->width >= q_h);

        u32 stride_img = img->width;
        Color color = q->GetColor();
        if (texture != NULL && has_tex_coords == true) {

            f32 q_scale_x = q->GetTextureScaleX(q_w);
            f32 q_scale_y = q->GetTextureScaleY(q_h);
            f32 q_u0 = q->GetTextureU0();
            f32 q_v0 = q->GetTextureV0();

            // i,j          : target coords
            // i_img, j_img : img coords

            for (u32 j = 0; j < q_h; ++j) {
                s32 j_img = j + q_y0;
                if (j_img < 0 || j_img > img->height) {
                    continue;
                }

                for (u32 i = 0; i < q_w; ++i) {
                    u32 i_img = q_x0 + i;
                    if (i_img < 0 || i_img > img->width) {
                        continue;
                    }
                    f32 x = q_u0 + i * q_scale_x;
                    f32 y = q_v0 + j * q_scale_y;
                    if (u8 b = SampleTexture(texture, x, y)) {
                        color.a = b;
                        u32 idx = j_img * stride_img + i_img;
                        img->img[idx] = color;
                    }
                }
            }
        }
        else {
            s32 j_img;
            u32 i_img;
            u32 idx;
            for (u32 j = 0; j < q_h; ++j) {
                j_img = j + q_y0;
                if (j_img < 0 || j_img > img->height) {
                    continue;
                }

                for (u32 i = 0; i < q_w; ++i) {
                    i_img = q_x0 + i;
                    if (i_img < 0 || i_img > img->width) {
                        continue;
                    }

                    idx = j_img * stride_img + i_img;
                    img->img[idx] = color;
                }
            }
        }
    }
}


//
// sprite renderer memory system


static ImageRGBA g_render_target;
static MArena _g_a_drawcalls;
static MArena *g_a_drawcalls;
static List<DrawCall> g_drawcalls;
static MArena _g_a_quadbuffer;
static MArena *g_a_quadbuffer;
static List<DrawCall> g_quadbuffer;
void SR_Clear() {
    ArenaClear(g_a_drawcalls);
    g_drawcalls = InitList<DrawCall>(g_a_drawcalls, 0);

    ArenaClear(g_a_quadbuffer);
    g_quadbuffer = InitList<DrawCall>(g_a_quadbuffer, 0);
}
void SR_Init(ImageRGBA render_target) {
    assert(render_target.img != NULL);
    g_render_target = render_target;

    if (g_a_drawcalls == NULL) {
        _g_a_drawcalls = ArenaCreate();
        g_a_drawcalls = &_g_a_drawcalls;
        g_drawcalls = InitList<DrawCall>(g_a_drawcalls, 0);

        _g_a_quadbuffer = ArenaCreate();
        g_a_quadbuffer = &_g_a_quadbuffer;
        g_quadbuffer = InitList<DrawCall>(g_a_quadbuffer, 0);
    }
    else {
        SR_Clear();
        printf("WARN: re-initialized sprite rendering\n");
    }
}
List<QuadHexaVertex> SR_Push(DrawCall dc) {
    ArenaAlloc(g_a_drawcalls, sizeof(DrawCall));
    g_drawcalls.Add(dc);
    return dc.quads;
}
void SR_Render() {
    assert(g_render_target.img != NULL && "init render target first");

    for (u32 i = 0; i < g_drawcalls.len; ++i) {
        BlitQuads(g_drawcalls.lst[i], &g_render_target);
    }
}


//
//  UI layout / non-text
//


struct Widget {
    s16 x0;
    s16 y0;
    s16 w;
    s16 h;
};
Widget InitWidget(s16 x0, s16 y0, s16 w, s16 h) {
    Widget wid;
    wid.x0 = x0;
    wid.y0 = y0;
    wid.w = w;
    wid.h = h;
    return wid;
}


List<QuadHexaVertex> LayoutPanel(
        MArena *a_dest,
        s32 l, s32 t, s32 w, s32 h,
        s32 border,
        Color col_border = { RGBA_GRAY_75 }, Color col_pnl = { RGBA_WHITE } )
{
    if (border >= w / 2 || border >= w / 2) {
        return List<QuadHexaVertex> { NULL, 0 };
    }

    DrawCall dc;
    dc.texture = 0;
    dc.quads = InitList<QuadHexaVertex>(a_dest, 2);
    {
        Quad q;
        _memzero(&q, sizeof(Quad));
        q.x0 = l;
        q.x1 = l + w;
        q.y0 = t;
        q.y1 = t + h;
        q.c = col_border;
        dc.quads.Add(QuadCook(q));
    }
    {
        Quad q;
        _memzero(&q, sizeof(Quad));
        q.x0 = l + border;
        q.x1 = l + w - border;
        q.y0 = t + border;
        q.y1 = t + h - border;
        q.c = col_pnl;
        dc.quads.Add(QuadCook(q));
    }

    List<QuadHexaVertex> quads = SR_Push(dc);
    return quads;
}
inline
List<QuadHexaVertex> LayoutPanel(
        s32 l, s32 t, s32 w, s32 h,
        s32 border,
        Color col_border = { RGBA_GRAY_75 }, Color col_pnl = { RGBA_WHITE } )
{
    return LayoutPanel(g_a_quadbuffer, l, t, w, h, border, col_border, col_pnl);
}


//
//  Text layout
//


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
List<QuadHexaVertex> LayoutText(MArena *a_dest, Str txt, s32 x0, s32 y0, s32 w, s32 h, Color color) {
    assert(g_text_plotter != NULL && "init text plotters first");
    GlyphPlotter *plt = g_text_plotter;

    s32 pt_x = x0;
    s32 pt_y = y0 + plt->ln_height;
    s32 box_r = x0 + w;
    s32 box_b = y0 + h;
    s32 w_space = plt->advance_x.lst[' '];

    u32 i = 0;
    Str s = txt;

    List<QuadHexaVertex> layed_out = InitList<QuadHexaVertex>(a_dest, 0);
    while (s.len > 0) {
        // while words

        if (IsNewLine(s)) {
            if (CanDoNewline(pt_y, plt->ln_height, plt->ln_ascend, box_b)) {
                DoNewLine(plt->ln_height, x0, &pt_x, &pt_y);
            }
            s = StrInc(s, 1);
        }
        if (IsWhiteSpace(s)) {
            if (CanDoWhiteSpace(pt_x, w_space, box_r)) {
                DoWhiteSpace(w_space, &pt_x);
            }
            s = StrInc(s, 1);
        }

        // lookahead word len (including any leading whitespace)
        u32 w_adv = 0;
        u32 w_len = WorldLen(s, plt->advance_x, &w_adv);

        // word wrap
        if (pt_x + w_adv > box_r) {
            if (CanDoNewline(pt_y, plt->ln_height, plt->ln_ascend, box_b)) {
                DoNewLine(plt->ln_height, x0, &pt_x, &pt_y);
            }
            else {
                // ran out of space, exit
                break;
                /*
                DrawCall dc;
                dc.texture = 0;
                dc.quads = layed_out;
                SR_Push(dc);
                return layed_out;
                */
            }
        }

        // lay out word
        for (u32 j = 0; j < w_len; ++j) {
            char c = s.str[j];
            QuadHexaVertex q = QuadOffset(plt->cooked.lst + c, pt_x, pt_y, color);
            pt_x += plt->advance_x.lst[c];
            ArenaAlloc(a_dest, sizeof(QuadHexaVertex));
            layed_out.Add(q);
        }
        s = StrInc(s, w_len);
    }
    assert(layed_out.len <= txt.len); // txt len minus whitespaces, which are not layed out as quads

    // only scale if absolutely necessary
    f32 scale = 1.0f;
    ScaleTextInline(layed_out, scale, x0, y0, w, h);

    DrawCall dc;
    dc.texture = 0;
    dc.quads = layed_out;
    SR_Push(dc);

    return layed_out;
}
inline
List<QuadHexaVertex> LayoutText(MArena *a_dest, const char *txt, s32 x0, s32 y0, s32 w, s32 h, Color color = { RGBA_BLACK }) {
    return LayoutText(a_dest, StrL(txt), x0, y0, w, h, color);
}
inline
List<QuadHexaVertex> LayoutText(Str txt, s32 x0, s32 y0, s32 w, s32 h, Color color = { RGBA_BLACK }) {
    return LayoutText(g_a_quadbuffer, txt, x0, y0, w, h, color);
}
inline
List<QuadHexaVertex> LayoutText(const char *txt, s32 x0, s32 y0, s32 w, s32 h, Color color = { RGBA_BLACK }, FontSize fs = FS_36) {
    FontSize org = GetFontSize();
    SetFontAndSize(fs);
    return LayoutText(g_a_quadbuffer, StrL(txt), x0, y0, w, h, color);
    SetFontAndSize(org);
}


#endif
