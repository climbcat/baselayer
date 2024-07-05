#ifndef __SPRITE_H__
#define __SPRITE_H__


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
    s32 GetX1() {
        s32 x1 = (s32) verts[0].pos.x;
        return x1;
    }
    inline
    s32 GetY1() {
        s32 y1 = (s32) verts[1].pos.y;
        return y1;
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
inline
void QuadOffset(QuadHexaVertex *q, f32 x, f32 y) {
    for (u32 i = 0; i < 6; ++i) {
        QuadVertex *v = q->verts + i;
        v->pos.x += x;
        v->pos.y += y;
    }
}


//
//  Sprite Rendering
//
//  DrawCall
//  SW Blitting


struct DrawCall {
    u32 texture;
    List<QuadHexaVertex> quads;
};
DrawCall InitDrawCall(List<QuadHexaVertex> quads, u32 texture) {
    DrawCall dc;
    dc.quads = quads;
    dc.texture = texture;
    return dc;
}


//
//  Texture and byte-texture mapping
//
static HashMap g_texb_map;
ImageB *GetGlyphAtlasByteTexture(u64 texture_id) {
    ImageB *result = (ImageB*) MapGet(&g_texb_map, texture_id);
    return result;
}
#define MAX_TEXTURE_B_CNT 1000
void InitSprites() {
    MContext *ctx = InitBaselayer();
    g_texb_map = InitMap(ctx->a_life, MAX_TEXTURE_B_CNT);
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
    ImageB *texture_b = GetGlyphAtlasByteTexture(call.texture);

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

        //
        // byte-texture / font glyph blitting
        //
        Color color_quad = q->GetColor();
        if (texture_b != NULL && has_tex_coords == true) {

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
                    if (u8 alpha_byte = SampleTexture(texture_b, x, y)) {
                        // rudimentary alpha-blending
                        u32 idx = j_img * stride_img + i_img;
                        Color color_background = img->img[idx];

                        f32 alpha = (1.0f * alpha_byte) / 255;
                        Color color_blended;
                        color_blended.r = floor( alpha*color_quad.r ) + floor( (1-alpha)*color_background.r );
                        color_blended.g = floor( alpha*color_quad.g ) + floor( (1-alpha)*color_background.g );
                        color_blended.b = floor( alpha*color_quad.b ) + floor( (1-alpha)*color_background.b );
                        color_blended.a = 255;

                        img->img[idx] = color_blended;
                    }
                }
            }
        }

        //
        // mono-color quads
        //
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
                    img->img[idx] = color_quad;
                }
            }
        }

        //
        // TODO: blit a full-color (32bit) texture
        //
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


#endif
