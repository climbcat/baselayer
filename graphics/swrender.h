#ifndef __SWRENDER_H__
#define __SWRENDER_H__


#include "../baselayer.h"
#include "geometry.h"
#include "entity.h"


//
// Screen coords


struct Vector2_u16 {
    u16 x;
    u16 y;
};
struct Vector2_s16 {
    s16 x;
    s16 y;
};
struct ScreenAnchor {
    s16 x;
    s16 y;
    Color c;
};
inline
bool CullScreenCoords(u32 pos_x, u32 pos_y, u32 w, u32 h) {
    bool not_result = pos_x >= 0 && pos_x < w && pos_y >= 0 && pos_y < h;
    return !not_result;
}
inline
u32 ScreenCoordsPosition2Idx(u32 pos_x, u32 pos_y, u32 width) {
    u32 result = pos_x + width * pos_y;
    return result;
}
inline
Vector2_s16 NDC2Screen(u32 w, u32 h, Vector3f ndc) {
    Vector2_s16 pos;

    pos.x = (s16) ((ndc.x + 1) / 2 * w);
    pos.y = (s16) ((ndc.y + 1) / 2 * h);

    return pos;
}
u16 LinesToScreenCoords(u32 w, u32 h, List<ScreenAnchor> *dest_screen_buffer, List<Vector2_u16> *index_buffer, List<Vector3f> *ndc_buffer, u32 lines_low, u32 lines_high, Color color) {
    u16 nlines_remaining = 0;
    for (u32 i = lines_low; i <= lines_high; ++i) {
        Vector2_u16 line = index_buffer->lst[i];
        Vector2_s16 a = NDC2Screen(w, h, ndc_buffer->lst[line.x]);
        Vector2_s16 b = NDC2Screen(w, h, ndc_buffer->lst[line.y]);

        if (CullScreenCoords(a.x, a.y, w, h) && CullScreenCoords(b.x, b.y, w, h)) {
            continue;
        }

        *(dest_screen_buffer->lst + dest_screen_buffer->len++) = ScreenAnchor { a.x, a.y, color };
        *(dest_screen_buffer->lst + dest_screen_buffer->len++) = ScreenAnchor { b.x, b.y, color };;
        nlines_remaining++;
    }
    return nlines_remaining;
}


//
// Render to image buffer


void RenderRandomPatternRGBA(u8* image_buffer, u32 w, u32 h) {
    RandInit();
    u32 pix_idx, r, g, b;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            pix_idx = j + i*w;
            r = RandIntMax(255);
            g = RandIntMax(255);
            b = RandIntMax(255);
            image_buffer[4 * pix_idx + 0] = r;
            image_buffer[4 * pix_idx + 1] = g;
            image_buffer[4 * pix_idx + 2] = b;
            image_buffer[4 * pix_idx + 3] = 255;
        }
    }
}
void RenderLineRGBA(u8* image_buffer, u16 w, u16 h, s16 ax, s16 ay, s16 bx, s16 by, Color color) {

    // initially working from a to b
    // there are four cases:
    // 1: slope <= 1, ax < bx
    // 2: slope <= 1, ax > bx 
    // 3: slope > 1, ay < by
    // 4: slope > 1, ay > by 

    f32 slope_ab = (f32) (by - ay) / (bx - ax);

    if (abs(slope_ab) <= 1) {
        // draw by x
        f32 slope = slope_ab;

        // swap?
        if (ax > bx) {
            u16 swapx = ax;
            u16 swapy = ay;

            ax = bx;
            ay = by;
            bx = swapx;
            by = swapy;
        }

        s16 x, y;
        u32 pix_idx;
        for (u16 i = 0; i <= bx - ax; ++i) {
            x = ax + i;
            y = ay + floor(slope * i);

            if (CullScreenCoords(x, y, w, h)) {
                continue;
            }

            pix_idx = x + y*w;
            image_buffer[4 * pix_idx + 0] = color.r;
            image_buffer[4 * pix_idx + 1] = color.g;
            image_buffer[4 * pix_idx + 2] = color.b;
            image_buffer[4 * pix_idx + 3] = color.a;
        }
    }
    else {
        // draw by y
        float slope_inv = 1 / slope_ab;

        // swap a & b ?
        if (ay > by) {
            u16 swapx = ax;
            u16 swapy = ay;

            ax = bx;
            ay = by;
            bx = swapx;
            by = swapy;
        }

        s16 x, y;
        u32 pix_idx;
        for (u16 i = 0; i <= by - ay; ++i) {
            y = ay + i;
            x = ax + floor(slope_inv * i);

            if (CullScreenCoords(x, y, w, h)) {
                continue;
            }

            pix_idx = x + y*w;
            image_buffer[4 * pix_idx + 0] = color.r;
            image_buffer[4 * pix_idx + 1] = color.g;
            image_buffer[4 * pix_idx + 2] = color.b;
            image_buffer[4 * pix_idx + 3] = color.a;
        }
    }
}
inline
void RenderLineRGBA(u8* image_buffer, u16 w, u16 h, ScreenAnchor a, ScreenAnchor b) {
    RenderLineRGBA(image_buffer, w, h, a.x, a.y, b.x, b.y, a.c);
}
void RenderPointCloud(u8* image_buffer, u16 w, u16 h, Matrix4f *mvp, List<Vector3f> points, Color color) {
    float color_fade = 1;
    Vector3f p;
    Vector3f p_ndc;
    Vector2_s16 p_screen;
    u32 pix_idx;
    for (u32 i = 0; i < points.len; ++i) {
        p = points.lst[i];
        p_ndc = TransformPerspective( *mvp, p );
        p_screen = NDC2Screen(w, h, p_ndc);
        if (CullScreenCoords(p_screen.x, p_screen.y, w, h) ) {
            continue;
        }
        pix_idx = ScreenCoordsPosition2Idx(p_screen.x, p_screen.y, w);
        image_buffer[4 * pix_idx + 0] = color.r * color_fade;
        image_buffer[4 * pix_idx + 1] = color.g * color_fade;
        image_buffer[4 * pix_idx + 2] = color.b * color_fade;
        image_buffer[4 * pix_idx + 3] = color.a;
    }
}
void RenderMesh() {
    printf("RenderMesh: not implemented\n");
}


//
// Rendering functions wrapper


struct SwRenderer {
    // settings
    u32 w;
    u32 h;
    float aspect;
    u32 nchannels;

    // pipeline buffers
    MArena _arena;
    MArena *a;
    List<Vector3f> vertex_buffer;
    List<Vector2_u16> index_buffer;
    List<Vector3f> ndc_buffer;
    List<ScreenAnchor> screen_buffer;
    u8 *image_buffer;

    // shader
    ScreenQuadTextureProgram screen;
};
SwRenderer InitRenderer(u32 width = 1280, u32 height = 800) {
    SwRenderer rend;
    rend.w = width;
    rend.h = height;
    rend.aspect = (float) width / height;
    u32 nchannels = 4;
    rend._arena = ArenaCreate();
    rend.a = &rend._arena;

    // pipeline buffers
    rend.image_buffer = (u8*) ArenaAlloc(rend.a, nchannels * rend.w * rend.h);
    rend.vertex_buffer = InitList<Vector3f>(rend.a, 1000);
    rend.index_buffer = InitList<Vector2_u16>(rend.a, 1000);
    rend.ndc_buffer = InitList<Vector3f>(rend.a, 1000);
    rend.screen_buffer = InitList<ScreenAnchor>(rend.a, 1000);

    // shader
    rend.screen.Init(rend.image_buffer, rend.w, rend.h);

    return rend;
}
void SwRenderFrame(SwRenderer *r, EntitySystem *es, Matrix4f *vp, u32 frameno) {
    TimeFunction;

    _memzero(r->image_buffer, 4 * r->w * r->h);
    r->screen_buffer.len = 0;

    // entity loop (POC): vertices -> NDC
    Matrix4f model, mvp;
    u32 eidx = 0;
    es->IterReset();
    Entity *next = es->IterNext();
    while (next != NULL) {
        if (next->active && (next ->data_tpe == EF_ANALYTIC)) {
            if (next->tpe == ET_LINES_ROT) {
                model = next->transform * TransformBuildRotateY(0.03f * frameno);
            }
            else {
                model = next->transform;
            }
            mvp = TransformBuildMVP(model, *vp);

            // lines to screen buffer
            for (u32 i = next->verts_low; i <= next->verts_high; ++i) {
                r->ndc_buffer.lst[i] = TransformPerspective(mvp, r->vertex_buffer.lst[i]);
                // TODO: here, it is faster to do a frustum filter in world space
            }
            // render lines
            LinesToScreenCoords(r->w, r->h, &r->screen_buffer, &r->index_buffer, &r->ndc_buffer, next->lines_low, next->lines_high, next->color);
        }
        else if (next->active && (next->data_tpe == EF_EXTERNAL)) {
            mvp = TransformBuildMVP(next->transform, *vp);

            // render pointcloud
            if (next->tpe == ET_POINTCLOUD) {
                RenderPointCloud(r->image_buffer, r->w, r->h, &mvp, next->GetVertices(), next->color);
            }
            else if (next->tpe == ET_MESH) {
                RenderMesh();
                // fallback, just render the vertices as a point cloud:
                RenderPointCloud(r->image_buffer, r->w, r->h, &mvp, next->GetVertices(), next->color);
            }
            else {
                printf("Unknown entity type: %d (external data type)\n", next->tpe);
            }
        }
        eidx++;
        next = es->IterNext();
    }
    r->ndc_buffer.len = r->vertex_buffer.len;
    for (u32 i = 0; i < r->screen_buffer.len / 2; ++i) {
        RenderLineRGBA(r->image_buffer, r->w, r->h, r->screen_buffer.lst[2*i + 0], r->screen_buffer.lst[2*i + 1]);
    }

    r->screen.Draw(r->image_buffer, r->w, r->h);
}


//
// Axis-aligned box


Entity InitAndActivateAABox(Vector3f translate_coords, float radius, SwRenderer *r) {
    Entity aabox = InitEntity(ET_LINES);
    aabox.transform = TransformBuild(y_hat, 0, translate_coords);
    aabox.color = { RGBA_GREEN };
    aabox.origo = Vector3f { 0, 0, 0 },
    aabox.dims = Vector3f { radius, radius, radius };

    // enter into the renderer
    Entity *box = &aabox;
    box->verts_low = r->vertex_buffer.len;
    box->lines_low = r->index_buffer.len;

    List<Vector3f> *verts_dest = &r->vertex_buffer;
    List<Vector2_u16> *idxs_dest = &r->index_buffer;
    u16 vertex_offset = verts_dest->len;
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 1) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 2) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 4) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 3), (u16) (vertex_offset + 1) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 3), (u16) (vertex_offset + 2) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 3), (u16) (vertex_offset + 7) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 5), (u16) (vertex_offset + 1) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 5), (u16) (vertex_offset + 4) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 5), (u16) (vertex_offset + 7) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 6), (u16) (vertex_offset + 2) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 6), (u16) (vertex_offset + 4) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset + 6), (u16) (vertex_offset + 7) };

    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y - box->dims.y, box->origo.z - box->dims.z };
    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y - box->dims.y, box->origo.z + box->dims.z };
    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y + box->dims.y, box->origo.z - box->dims.z };
    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y + box->dims.y, box->origo.z + box->dims.z };
    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y - box->dims.y, box->origo.z - box->dims.z };
    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y - box->dims.y, box->origo.z + box->dims.z };
    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y + box->dims.y, box->origo.z - box->dims.z };
    *(verts_dest->lst + verts_dest->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y + box->dims.y, box->origo.z + box->dims.z };

    box->verts_high = r->vertex_buffer.len - 1;
    box->lines_high = r->index_buffer.len - 1;

    return aabox;
}
Entity *InitAndActivateAABox(EntitySystem *es, Vector3f center_transf, float radius, SwRenderer *r) {
    Entity *box = es->AllocEntity();
    *box = InitAndActivateAABox(center_transf, radius, r);
    EntitySystemChain(es, box);
    return box;
}


//
// Coordinate axes


Entity InitAndActivateCoordAxes(SwRenderer *r) {
    Entity ax = InitEntity(ET_AXES);
    ax.color = { RGBA_BLUE };
    ax.origo = { 0, 0, 0 };
    ax.dims = { 1, 1, 1 };

    // enter into the renderer
    Entity *axes = &ax;
    axes->verts_low = r->vertex_buffer.len;
    axes->lines_low = r->index_buffer.len;

    Vector3f x { 1, 0, 0 };
    Vector3f y { 0, 1, 0 };
    Vector3f z { 0, 0, 1 };

    List<Vector3f> *verts_dest = &r->vertex_buffer;
    List<Vector2_u16> *idxs_dest = &r->index_buffer;
    u16 vertex_offset = verts_dest->len;
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset), (u16) (vertex_offset + 1) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset), (u16) (vertex_offset + 2) };
    *(idxs_dest->lst + idxs_dest->len++) = Vector2_u16 { (u16) (vertex_offset), (u16) (vertex_offset + 3) };
    
    *(verts_dest->lst + verts_dest->len++) = (axes->origo);
    *(verts_dest->lst + verts_dest->len++) = (axes->origo + x);
    *(verts_dest->lst + verts_dest->len++) = (axes->origo + y);
    *(verts_dest->lst + verts_dest->len++) = (axes->origo + z);

    axes->verts_high = r->vertex_buffer.len - 1;
    axes->lines_high = r->index_buffer.len - 1;

    return ax;
}
Entity *InitAndActivateCoordAxes(EntitySystem *es, SwRenderer *r) {
    Entity *axes = es->AllocEntity();
    *axes = InitAndActivateCoordAxes(r);
    EntitySystemChain(es, axes);
    return axes;
}


//
// Point cloud


Entity *InitAndActivateDataEntity(EntitySystem *es, SwRenderer *r, MArena *a, StreamType tpe, u32 npoints_max, u32 id, StreamHeader *prev) {
    StreamHeader *hdr = InitStream(a, tpe, npoints_max, prev);
    hdr->id = id;
    Entity *pc = es->AllocEntity();
    pc->data_tpe = EF_EXTERNAL;
    pc->tpe = ET_POINTCLOUD;
    pc->entity_stream = hdr;
    pc->color  = { RGBA_GREEN };
    pc->transform = hdr->transform;
    EntitySystemChain(es, pc);
    return pc;
}

Entity *LoadAndActivateDataEntity(EntitySystem *es, SwRenderer *r, StreamHeader *data, bool do_transpose) {
    Entity *ent = es->AllocEntity();
    ent->data_tpe = EF_EXTERNAL;
    if (data->tpe == ST_POINTS) {
        ent->tpe = ET_POINTCLOUD;
        ent->color  = { RGBA_GREEN };
        if (do_transpose == true) {
            ent->transform = Matrix4f_Transpose( data->transform );
        }
        else {
            ent->transform = data->transform;
        }
    }
    else if (data->tpe == ST_VERTICES || ST_NORMALS || ST_INDICES3) {
        ent->tpe = ET_MESH;
        ent->color  = { RGBA_BLUE };
        ent->transform = data->transform;
    }
    ent->entity_stream = data;

    EntitySystemChain(es, ent);
    return ent;
}


#endif
