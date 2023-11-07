#ifndef __SWRENDER_H__
#define __SWRENDER_H__


//
// Colors


#define RGBA_BLACK      0, 0, 0, 255
#define RGBA_WHITE      255, 255, 255, 255
#define RGBA_RED        255, 0, 0, 255
#define RGBA_GREEN      0, 255, 0, 255
#define RGBA_BLUE       0, 0, 255, 255
struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};
void PrintColorInline(Color c) {
    printf("%hhx %hhx %hhx %hhx", c.r, c.g, c.b, c.a);
}


//
// Screen coords


struct Vector2_u16 {
    u16 x;
    u16 y;
};
struct ScreenAnchor {
    u16 x;
    u16 y;
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
Vector2_u16 NDC2Screen(u32 w, u32 h, Vector3f ndc) {
    Vector2_u16 pos;

    pos.x = (u32) ((ndc.x + 1) / 2 * w);
    pos.y = (u32) ((ndc.y + 1) / 2 * h);

    return pos;
}
u16 LinesToScreenCoords(u32 w, u32 h, List<ScreenAnchor> *dest_screen_buffer, List<Vector2_u16> *index_buffer, List<Vector3f> *ndc_buffer, u32 lines_low, u32 lines_high, Color color) {
    u16 nlines_remaining = 0;
    for (u32 i = lines_low; i <= lines_high; ++i) {
        Vector2_u16 line = index_buffer->lst[i];
        Vector2_u16 a = NDC2Screen(w, h, ndc_buffer->lst[line.x]);
        Vector2_u16 b = NDC2Screen(w, h, ndc_buffer->lst[line.y]);

        // TODO: crop to NDC box
        if (CullScreenCoords(a.x, a.y, w, h) || CullScreenCoords(b.x, b.y, w, h)) {
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
void RenderLineRGBA(u8* image_buffer, u16 w, u16 h, u16 ax, u16 ay, u16 bx, u16 by, Color color) {

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

        u16 x, y;
        u32 pix_idx;
        for (u16 i = 0; i <= bx - ax; ++i) {
            x = ax + i;
            y = ay + floor(slope * i);

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

        u16 x, y;
        u32 pix_idx;
        for (u16 i = 0; i <= by - ay; ++i) {
            y = ay + i;
            x = ax + floor(slope_inv * i);

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
    for (u32 i = 0; i < points.len; ++i) {

        Vector3f p_ndc = TransformPerspective( *mvp, points.lst[i] );
        Vector2_u16 p_screen = NDC2Screen(w, h, p_ndc);
        if (CullScreenCoords(p_screen.x, p_screen.y, w, h) ) {
            continue;
        }
        u32 pix_idx = ScreenCoordsPosition2Idx(p_screen.x, p_screen.y, w);
        image_buffer[4 * pix_idx + 0] = color.r;
        image_buffer[4 * pix_idx + 1] = color.g;
        image_buffer[4 * pix_idx + 2] = color.b;
        image_buffer[4 * pix_idx + 3] = color.a;
    }
}

 
//
// Entity System


enum EntityType {
    ET_AXES,
    ET_LINES,
    ET_LINES_ROT,
    ET_STREAMDATA,

    ET_COUNT,
};
enum EntityDataType {
    DT_VERTICES,
    DT_NORMALS,
    DT_INDICES2,
    DT_INDICES3,
    DT_RGBA, // bitmap (4B stride)
    DT_F32, // depth image (4B stride)
    DT_TEXCOORDS, // [0;1]^2 image, (8B stride)

    DT_COUNT,
};
struct Vector3i {
    u32 i1;
    u32 i2;
    u32 i3;
};
struct Vector2i {
    u32 i1;
    u32 i2;
};
struct Vector2f {
    f32 x;
    f32 y;
};

struct ImageF32 { // e.g. a depth image
    u32 width;
    u32 height;
    f32 *data;
};
struct ImageU32 { // e.g. a bit map
    u32 width;
    u32 height;
    f32 *data;
};
struct EntityStream {
    u32 next; // purpose: byte offset for iterating the chunk, zero indicates the final entry
    u32 id; // purpose: for associating different EntityStream entries with the same object within the chunk (e.g. depth + colour)
    u32 time; // purpose: real-time data header info for sorting & filtering by post- or external process
    // TODO: just have payload_size; plus a GetPointCount() method that translates into the current units
    u32 datasize; // payload size
    u32 linesize; // line size for 2d data
    Matrix4f transform; // purpose: storage of this ever-present header info
    EntityDataType tpe; // purpose: allows enterpritation of data payload

    //
    // utility functions

    EntityStream *GetNext() {
        EntityStream *result = NULL;
        if (next) {
            result = (EntityStream *) ((u8*) this + next);
        }
        return result;
    }
    u8 *GetData() {
        // returns a pointer to where the data is supposed to start (right after this struct's location in memory)
        return (u8*) this + sizeof(EntityStream);
    }
    u32 GetDatumCount() {
        // returns data count in iteration-ready units of 4B / 8B
        // TODO: switch by type
        return datasize / sizeof(Vector3f);
    }
    List<Vector3f> GetDataVector3f() {
        List<Vector3f> data;
        data.lst = (Vector3f*) GetData();
        data.len = GetDatumCount();
        return data;
    }
    void SetVertexCount(u32 npoints) {
        datasize = npoints * sizeof(Vector3f);
    }
    List<Vector3i> GetDataVector3i() {
        List<Vector3i> data;
        data.lst = (Vector3i*) GetData();
        data.len = GetDatumCount();
        return data;
    }
    u32 Get2DWidth() {
        // returns data width (line width) in iteration-ready units of 4B / 8B
        // TODO: switch by type
        return linesize / sizeof(Vector3f);
    }
    u32 GetDatumSize() {
        // TODO: switch by type
        return sizeof(Vector3f);
    }
    ImageU32 GetDataRGBA();
    ImageF32 GetDataF32();
    List<Vector2f> GetDataTexCoords();

    // TODO: Q. How will EntityStream get ported to OGL? Using one VBO with DrawIndices, one VBO pr. stream, or something else?
    //      Sbould data structure or EntityStream be redone to accomodate easy uploading of its data?
    // NOTE: Entity is intended to become a mem-pooled scene graph node type, EntityStream an external, serializable
    //      data storage system. Entity should be serializable as well, maybe as an EntityStream data type, but
    //      first is must be pooled and id-based for its references, u16s that do next, parent, children
};
Vector3f PointCloudAverage(Vector3f *data, u32 npoints, bool nonzero_only) {
    Vector3f result = Vector3f_Zero();
    Vector3f v;
    Vector3f zero = Vector3f_Zero();
    u32 cnt = 0;
    for (u32 i = 0; i < npoints; ++i) {
        v = data[i];
        if (nonzero_only && (v == zero)) {
            continue;
        }
        ++cnt;
        result = result + v;
    }
    f32 factor = 0;
    if (cnt > 0) {
        factor = 1.0f / cnt;
    }
    result.x *= factor;
    result.y *= factor;
    result.z *= factor;
    return result;
}
void EntityStreamPrint(EntityStream *et, char *tag) {
    printf("data: %s \n", tag);
    u32 didx = 0;
    while (et) {
        printf("%u: data type: %d carries %d bytes", didx, et->tpe, et->datasize);
        Vector3f av = PointCloudAverage((Vector3f*) et->GetData(), et->GetDatumCount(), true);
        printf(" av. %f %f %f\n", av.x, av.y, av.z);
        et = et->GetNext();
        ++didx;
    }
}
EntityStream *InitEntityStream(MArena *a, EntityDataType tpe, u32 npoints_max, EntityStream *prev) {
    // NOTE: next is safely set on prev here
    EntityStream *es = (EntityStream*) ArenaAlloc(a, sizeof(EntityStream), true);
    es->next = 0;
    es->id = 0;
    es->time = 0;
    es->SetVertexCount(npoints_max);
    es->linesize = 0;
    es->transform = Matrix4f_Identity();
    es->tpe = tpe;

    // linked list pointer is assigned on prevíous element
    if (prev != NULL) {
        assert(es > prev && "InitEntityStream");
        prev->next = (u8*) es - (u8*) prev;
    }

    // TODO: switch by data type to determine allocation size
    List<Vector3f> points = InitList<Vector3f>(a, npoints_max);
    assert((Vector3f*) points.lst == (Vector3f*) es->GetData() && "InitEntityStream");

    return es;
}
u32 PointerDiff(void *from, void *to) {
    assert(from < to && "PointerDiff");
    u32 result = (u8*) to - (u8*) from;
    return result;
}
u8 *PointerOffsetBytes(void *ptr, int offset) {
    assert(ptr != NULL && "PointerOffsetBytes");
    u8 *result = (u8*) ptr + offset;
    return result;
}
void EntityStreamFinalize(MArena *a, u32 npoints_actual, EntityStream *hdr) {
    // NOTE: next is not set at this point
    u32 offset = npoints_actual * hdr->GetDatumSize();
    u8 *hdr_next = PointerOffsetBytes(hdr, offset);
    u8 *arena_current = a->mem + a->used;
    a->used = a->used - PointerDiff( hdr_next, arena_current );
    hdr->SetVertexCount(npoints_actual);
}
struct Entity {
    // TODO: move to using ids / idxs rather than pointers. These can be u16
    Entity *next = NULL;
    EntityType tpe;
    Matrix4f transform; // TODO: make this into a transform_id and store externally
    Color color { RGBA_WHITE };

    // analytic entity indices (kept in a single "vbo" within the renderer)
    u16 verts_low = 0;
    u16 verts_high = 0;
    u16 lines_low = 0;
    u16 lines_high = 0;

    // analytic entity parameters (AABox, CoordAxes)
    Vector3f origo;
    Vector3f dims;
    inline float XMin() { return origo.x - dims.x; }
    inline float XMax() { return origo.x + dims.x; }
    inline float YMin() { return origo.y - dims.y; }
    inline float YMax() { return origo.y + dims.y; }
    inline float ZMin() { return origo.z - dims.z; }
    inline float ZMax() { return origo.z + dims.z; }
    inline bool FitsWithinBox(Vector3f v) {
        bool result =
            XMin() <= v.x && XMax() >= v.x &&
            YMin() <= v.y && YMax() >= v.y &&
            ZMin() <= v.z && ZMax() >= v.z;
        return result;
    }

    // data entity parameters (pointcloud, ...)
    EntityStream *entity_stream;
    List<u8> GetData() {
        List<u8> data;
        if (entity_stream != NULL) {
            data.lst = entity_stream->GetData();
            data.len = entity_stream->datasize;
        }
        return data;
    }
    List<Vector3f> GetVertices() {
        List<Vector3f> verts;
        if (entity_stream != NULL) {
            verts = entity_stream->GetDataVector3f();
        }
        return verts;
    }

    // scene graph behavior
    bool active = true;
    void Activate() {
        active = true;
    }
    void DeActivate() {
        active = false;
    }
};
Entity InitEntity(EntityType tpe) {
    Entity e;
    e.next = NULL;
    e.tpe = tpe;
    e.transform = Matrix4f_Identity();
    e.active = true;
    return e;
}


//
// Entity System organizes into a linked list / tree

#define ENTITY_MAX_COUNT 200
struct EntitySystem {
    MPool pool;
    Entity *first = NULL;
    Entity *next = NULL;
    Entity *last = NULL;
    void IterReset() {
        next = first;
    }
    Entity *IterNext() {
        Entity *result = next;
        if (next != NULL) {
            next = next->next;
        }
        return result;
    }
    Entity *AllocEntity() {
        Entity *result = (Entity*) PoolAlloc(&pool);
        return result;
    }
};
EntitySystem InitEntitySystem() {
    EntitySystem es;
    es.pool = PoolCreate(sizeof(Entity), ENTITY_MAX_COUNT);
    return es;
}
void EntitySystemChain(EntitySystem *es, Entity *next) {
    if (es->first == NULL) {
        es->first = next;
        es->last = next;
        es->IterReset();
    }
    else {
        es->last->next = next;
        es->last = next;
    }
}
void EntitySystemPrint(EntitySystem *es) {
    u32 eidx = 0;
    Entity *next = es->first;
    printf("entities: \n");
    while (next != NULL) {
        if (next->tpe != ET_STREAMDATA) {
            printf("%u: analytic, vertices %u -> %u lines %u -> %u\n", eidx, next->verts_low, next->verts_high, next->lines_low, next->lines_high);
        }
        else {
            printf("%u: data#%d, %u bytes\n", eidx, next->entity_stream->tpe, next->entity_stream->datasize);
        }
        eidx++;
        next = next->next;
    }
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
    _memzero(r->image_buffer, 4 * r->w * r->h);
    r->screen_buffer.len = 0;

    // entity loop (POC): vertices -> NDC
    Matrix4f model, mvp;
    u32 eidx = 0;
    es->IterReset();
    Entity *next = es->IterNext();
    while (next != NULL) {
        if (next ->tpe != ET_STREAMDATA) {
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
        else {
            mvp = *vp;

            // render pointcloud
            RenderPointCloud(r->image_buffer, r->w, r->h, &mvp, next->GetVertices(), next->color);
        }
        eidx++;
        next = next->next;
    }
    r->ndc_buffer.len = r->vertex_buffer.len;
    for (u32 i = 0; i < r->screen_buffer.len / 2; ++i) {
        RenderLineRGBA(r->image_buffer, r->w, r->h, r->screen_buffer.lst[2*i + 0], r->screen_buffer.lst[2*i + 1]);
    }

    r->screen.Draw(r->image_buffer, r->w, r->h);
}


//
// Game / frame loop wrapper


struct GameLoopOne {
    u64 frameno;
    SDL_Window *window; // <- TODO: replace with glfw
    SwRenderer renderer;
    MouseTrap mouse;
    OrbitCamera cam;

    // as pointer
    inline
    SwRenderer *GetRenderer() {
        return &renderer;
    }
    inline
    MouseTrap *GetMouseTrap()  {
        return &mouse;
    }
    inline
    OrbitCamera *GetOrbitCam() {
        return &cam;
    }
    bool GameLoopRunning() {
        return Running(&mouse);
    }
    void CycleFrame(EntitySystem *es) {
        // this frame
        cam.Update(&mouse, true);
        SwRenderFrame(&renderer, es, &cam.vp, frameno);
        SDL_GL_SwapWindow(window);
        frameno++;

        // start of next frame
        XSleep(10);
    }
};
GameLoopOne InitGameLoopOne(u32 width, u32 height) {
    GameLoopOne gl;
    gl.frameno = 0;
    gl.window = InitSDL(width, height, false);
    gl.renderer = InitRenderer(width, height);
    gl.cam = InitOrbitCamera(gl.renderer.aspect);
    gl.mouse = InitMouseTrap();
    return gl;
}


//
// Axis-aligned box


Entity InitAndActivateAABox(Vector3f center_transf, float radius, SwRenderer *r) {
    Entity aabox = InitEntity(ET_LINES);
    aabox.transform = TransformBuild(y_hat, 0, center_transf);
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


Entity *InitAndActivateDataEntity(EntitySystem *es, SwRenderer *r, MArena *a, EntityDataType tpe, u32 npoints_max, u32 id, EntityStream *prev) {
    EntityStream *hdr = InitEntityStream(a, tpe, npoints_max, prev);
    hdr->id = id;
    Entity *pc = es->AllocEntity();
    pc->tpe = ET_STREAMDATA;
    pc->entity_stream = hdr;
    pc->color  = { RGBA_GREEN };
    pc->transform = hdr->transform;
    EntitySystemChain(es, pc);
    return pc;
}


#endif
