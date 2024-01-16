#ifndef __ENTITYSYSTEM_H__
#define __ENTITYSYSTEM_H__


#include "../baselayer.h"
#include "stream.h"
#include "geometry.h"


//
// Colors


#define RGBA_BLACK      0, 0, 0, 255
#define RGBA_WHITE      255, 255, 255, 255
#define RGBA_GRAY_50    128, 128, 128, 255
#define RGBA_GRAY_25    64, 64, 64, 255
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
// Entity System


enum EntityType {
    ET_AXES,
    ET_LINES,
    ET_LINES_ROT,
    ET_POINTCLOUD,
    ET_POINTCLOUD_W_NORMALS,
    ET_MESH,
    ET_EMPTY_NODE,
    ET_ANY,

    ET_CNT,
};
enum EntityTypeFilter {
    EF_ANALYTIC,
    EF_STREAM,
    EF_EXTERNAL,
    EF_ANY,

    EF_CNT,
};


//
// Entity


struct Entity {
    Entity *up = NULL;
    Entity *down = NULL;
    Entity *prev = NULL;
    Entity *next = NULL;

    EntityTypeFilter data_tpe;
    EntityType tpe;
    Matrix4f transform; // TODO: make this into a transform_id and store externally
    Color color { RGBA_WHITE };
    Color color_alt { RGBA_WHITE };

    // analytic entity indices (kept in a single "vbo" within the renderer)
    u16 verts_low = 0;
    u16 verts_high = 0;
    u16 lines_low = 0;
    u16 lines_high = 0;

    // utility fields
    Vector3f origo;
    Vector3f dims;

    // stream data
    StreamHeader *entity_stream;

    // external data
    List<Vector3f> *ext_points;
    List<Vector3f> *ext_normals;
    List<Vector3f> ext_points_lst;
    List<Vector3f> ext_normals_lst;

    // scene graph switch
    bool active = true;

    // transforms
    inline Matrix4f GetLocal2World() {
        return transform;
    }
    inline Matrix4f GetWorld2Local() {
        return TransformGetInverse(&transform);
    }

    // analytic entity parameters (AABox, CoordAxes)
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

    List<Vector3f> GetVertices() {
        assert(
            (data_tpe == EF_STREAM || data_tpe == EF_EXTERNAL) &&
            (tpe == ET_POINTCLOUD || tpe == ET_POINTCLOUD_W_NORMALS) && "GetVertices: Only call with point cloud or mesh ext data"
        );

        List<Vector3f> verts;
        if (data_tpe == EF_STREAM)  {
            if (entity_stream != NULL) {
                verts = entity_stream->GetDataVector3f();
            }
        }
        else if (data_tpe == EF_EXTERNAL) {
            verts = *ext_points;
        }
        return verts;
    }
    List<Vector3f> GetNormals() { 
        // TODO: handle the situation where normals are located in stream->nxt location

        List<Vector3f> normals { NULL, 0 };
        if (ext_normals != NULL) {
            normals = *ext_normals;
        }
        return normals;
    }
    void SetStreamVertexCount(u32 npoints) {
        assert(data_tpe == EF_STREAM && (tpe == ET_POINTCLOUD || tpe == ET_POINTCLOUD_W_NORMALS || tpe == ET_MESH));
        if (entity_stream->tpe == ST_POINTS) {
            entity_stream->SetVertexCount(npoints);
        }
        else {
            assert(1 == 0 && "warning, inconsistent use of SetStreamVertexCount");
        }
        ext_points_lst = entity_stream->GetDataVector3f();
        ext_points = &ext_points_lst;
    }
    void SetStreamNormalsCount(u32 npoints) {
        assert(data_tpe == EF_STREAM && (tpe == ET_POINTCLOUD_W_NORMALS || tpe == ET_MESH));
        StreamHeader *nxt = entity_stream->GetNext(true);
        if (nxt->tpe == ST_NORMALS) {
            nxt->SetVertexCount(npoints);
        }
        else {
            assert(1 == 0 && "warning, unrestricted use of SetStreamNormalsCount");
        }
        ext_normals_lst = nxt->GetDataVector3f();
        ext_normals = &ext_normals_lst;
    }

    // scene graph behavior
    void Activate() {
        active = true;
    }
    void DeActivate() {
        active = false;
    }
};
static Entity entity_zero;
Entity InitEntity(EntityType tpe) {
    Entity e;
    e.tpe = tpe;
    if (tpe == ET_MESH || tpe == ET_POINTCLOUD) {
        e.data_tpe = EF_STREAM;
    }
    else {
        e.data_tpe = EF_ANALYTIC;
    }
    e.transform = Matrix4f_Identity();
    e.active = true;
    return e;
}
void EntityCopyBodyOnly(Entity *dest, Entity body) {
    Entity hdr = *dest;
    *dest = body;
    dest->up = hdr.up;
    dest->down = hdr.down;
    dest->prev = hdr.prev;
    dest->next = hdr.next;
}
void EntityYank(Entity *ent) {
    if (ent->next) {
        ent->next->prev = ent->prev;
    }
    if (ent->prev) {
        ent->prev->next = ent->next;
    }
    if (ent->up) {
        ent->up->down = ent->down;
    }
    if (ent->down) {
        ent->down->up = ent->up;
    }
    ent->up = NULL;
    ent->down = NULL;
    ent->prev = NULL;
    ent->next = NULL;
}
void EntityInsertAbove(Entity *ent_new, Entity *ent) {
    // inserts ent_new above ent
    EntityYank(ent_new);
    ent_new->up = ent->up;
    ent_new->down = ent;
    if (ent->up) {
        ent->up->down = ent_new;
    }
    ent->up = ent_new;
}
void EntityInsertBelow(Entity *ent_new, Entity *ent) {
    // inserts ent_new below ent
    EntityYank(ent_new);
    ent_new->up = ent;
    ent_new->down = ent->down;
    if (ent->down) {
        ent->down->up = ent_new;
    }
    ent->down = ent_new;
}
void EntityInsertBefore(Entity *ent_new, Entity *ent) {
    // inserts ent_new before ent
    EntityYank(ent_new);
    ent_new->prev = ent->prev;
    ent_new->next = ent;
    if (ent->prev) {
        ent->prev->next = ent_new;
    }
    ent->prev = ent_new;
}
void EntityInsertAfter(Entity *ent_new, Entity *ent) {
    // inserts ent_new after ent
    EntityYank(ent_new);
    ent_new->prev = ent;
    ent_new->next = ent->next;
    if (ent->next) {
        ent->next->prev = ent_new;
    }
    ent->next = ent_new;
}


//
// Organization of entities


#define TREE_ITER_STACK_CAPACITY 1000
struct TreeIterState {
    Entity* mem[TREE_ITER_STACK_CAPACITY];
    Stack<Entity*> _stc;
    inline
    void Push(Entity* item) {
        _stc.Push(item);
    }
    inline
    Entity* Pop() {
        return _stc.Pop();
    }
};
void InitTreeIter(TreeIterState *iter) {
    iter->_stc.lst = &iter->mem[0];
    iter->_stc.cap = TREE_ITER_STACK_CAPACITY;
}


#define ENTITY_MAX_CNT 2000
struct EntitySystem {
    MPool pool;
    Entity *root = NULL;
    Entity *leaf = NULL;

    Entity *cursor = NULL;
    Color cursor_highlight_color;
    Color cursor_color_swap;
    bool cursor_active_swap;

    Entity *TreeIterNext(Entity *prev, TreeIterState *iter, bool only_active_entities = true) {
        // descent is halted if active == false
        if (prev == NULL) {
            return root;
        }
        if (prev->next) {
            iter->Push(prev->next);
        }
        if (prev->down && (prev->active || !only_active_entities)) {
            return prev->down;
        }
        return iter->Pop();
    }
    bool FreeEntity(Entity *e) {
        if (e == cursor) {
            cursor = NULL;
        }
        if (e == root) {
            root = NULL;
        }
        if (e == leaf) {
            leaf = NULL;
        }
        EntityYank(e);
        return PoolFree(&pool, e);
    }
    void FreeEntityBranch(Entity *branch_root) {
        if (branch_root) {
            Entity *ent = branch_root;
            Entity *next;
            TreeIterState itr;
            InitTreeIter(&itr);
            while (ent) {
                next = TreeIterNext(ent, &itr);
                FreeEntity(ent);
                ent = next;
            }
        }
    }
    Entity *AllocEntity() {
        Entity default_val;
        Entity *next = (Entity*) PoolAlloc(&pool);
        assert(next != NULL && "pool capacity exceeded");
        *next = default_val;
        return next;
    }
    Entity *AllocEntityChain(Entity *at = NULL, bool below = false) {
        Entity *next = AllocEntity();
        if (root == NULL) {
            this->root = next;
            this->leaf = next;
        }
        else {
            Entity *after = at ? at : (this->leaf ? this->leaf : this->root); // assign at, leaf or root
            if (below == false) {
                EntityInsertAfter(next, after);
            }
            else {
                EntityInsertBelow(next, after);
            }
            this->leaf = next;
        }
        return next;
    }

    // TODO: improve this "cursed" code by applying zero-is-initialization
    void CursorMove(Entity *ent) {
        if (!ent) {
            return;
        }
        if (cursor != ent) {
            cursor->color = cursor_color_swap;
            cursor->active = cursor_active_swap;
        }
        cursor_color_swap = ent->color;
        cursor_active_swap = ent->active;
        ent->color = cursor_highlight_color;
        ent->active = true;
        cursor = ent;
    }
    inline
    void CursorInit() {
        if (!cursor) {
            cursor = root;
            cursor_color_swap = root->color;
            cursor_active_swap = root->active;
        }
    }
    void CursorDisable() {
        cursor = NULL;
    }
    void CursorToggleEntityActive() {
        CursorInit();
        cursor_active_swap = !cursor_active_swap;
    }
    void CursorPrev() {
        CursorInit();
        CursorMove(cursor->prev);
    }
    void CursorNext() {
        CursorInit();
        CursorMove(cursor->next);
    }
    void CursorUp() {
        CursorInit();
        CursorMove(cursor->up);
    }
    void CursorDown() {
        CursorInit();
        if (cursor_active_swap) {
            CursorMove(cursor->down);
        }
    }
};


void EntitySystemPrint(EntitySystem *es) {
    u32 eidx = 0;
    printf("entities: \n");
    TreeIterState iter;
    InitTreeIter(&iter);
    Entity *next = es->TreeIterNext(NULL, &iter, false);
    while (next != NULL) {
        if (next->data_tpe == EF_ANALYTIC) {
            printf("%u: analytic,   vertices %u -> %u lines %u -> %u\n", eidx, next->verts_low, next->verts_high, next->lines_low, next->lines_high);
        }
        else if (next->data_tpe == EF_STREAM) {
            printf("%u: stream#%d,  %u bytes\n", eidx, next->entity_stream->tpe, next->entity_stream->datasize);
        }
        else if (next->data_tpe == EF_EXTERNAL) {
            printf("%u: pointcloud, %lu bytes\n", eidx, next->ext_points->len * sizeof(Vector3f));
        }
        eidx++;
        next = es->TreeIterNext(next, &iter, false);
    }
}

static EntitySystem _g_entity_system;
static EntitySystem *g_entity_system;
EntitySystem *InitEntitySystem() {
    assert(g_entity_system == NULL && "singleton assert");
    g_entity_system = &_g_entity_system;

    g_entity_system->pool = PoolCreate(sizeof(Entity), ENTITY_MAX_CNT);
    g_entity_system->cursor_highlight_color = Color { RGBA_WHITE };
    void *zero = PoolAlloc(&g_entity_system->pool);
    return g_entity_system;
}


//
// Coordinate axes


Entity CoordAxes(List<Vector3f> *vertex_buffer, List<Vector2_u16> *index_buffer) {
    Entity ax = InitEntity(ET_AXES);
    ax.color = { RGBA_BLUE };
    ax.origo = { 0, 0, 0 };
    ax.dims = { 1, 1, 1 };
    Vector3f x { 1, 0, 0 };
    Vector3f y { 0, 1, 0 };
    Vector3f z { 0, 0, 1 };

    Entity *axes = &ax;
    axes->verts_low = vertex_buffer->len;
    axes->lines_low = index_buffer->len;

    u16 vertex_offset = vertex_buffer->len;
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset), (u16) (vertex_offset + 1) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset), (u16) (vertex_offset + 2) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset), (u16) (vertex_offset + 3) };
    *(vertex_buffer->lst + vertex_buffer->len++) = (axes->origo);
    *(vertex_buffer->lst + vertex_buffer->len++) = (axes->origo + x);
    *(vertex_buffer->lst + vertex_buffer->len++) = (axes->origo + y);
    *(vertex_buffer->lst + vertex_buffer->len++) = (axes->origo + z);

    axes->verts_high = vertex_buffer->len - 1;
    axes->lines_high = index_buffer->len - 1;

    return ax;
}


//
// Axis-aligned box


Entity AABox(Vector3f translate_coords, float radius, List<Vector3f> *vertex_buffer, List<Vector2_u16> *index_buffer) {
    Entity aabox = InitEntity(ET_LINES);
    aabox.transform = TransformBuild(y_hat, 0, translate_coords);
    aabox.color = { RGBA_GREEN };
    aabox.origo = Vector3f { 0, 0, 0 },
    aabox.dims = Vector3f { radius, radius, radius };

    Entity *box = &aabox;
    box->verts_low = vertex_buffer->len;
    box->lines_low = index_buffer->len;

    u16 vertex_offset = vertex_buffer->len;
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 1) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 2) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 4) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 3), (u16) (vertex_offset + 1) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 3), (u16) (vertex_offset + 2) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 3), (u16) (vertex_offset + 7) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 5), (u16) (vertex_offset + 1) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 5), (u16) (vertex_offset + 4) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 5), (u16) (vertex_offset + 7) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 6), (u16) (vertex_offset + 2) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 6), (u16) (vertex_offset + 4) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 6), (u16) (vertex_offset + 7) };

    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y - box->dims.y, box->origo.z - box->dims.z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y - box->dims.y, box->origo.z + box->dims.z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y + box->dims.y, box->origo.z - box->dims.z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x - box->dims.x, box->origo.y + box->dims.y, box->origo.z + box->dims.z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y - box->dims.y, box->origo.z - box->dims.z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y - box->dims.y, box->origo.z + box->dims.z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y + box->dims.y, box->origo.z - box->dims.z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x + box->dims.x, box->origo.y + box->dims.y, box->origo.z + box->dims.z };

    box->verts_high = vertex_buffer->len - 1;
    box->lines_high = index_buffer->len - 1;

    return aabox;
}


//
// Camera wireframe


Entity CameraWireframe(float radius_xy, float length_z, List<Vector3f> *vertex_buffer, List<Vector2_u16> *index_buffer) {
    Entity cam = InitEntity(ET_LINES);
    cam.color = { RGBA_WHITE };
    cam.origo = Vector3f { 0, 0, 0 },
    cam.dims = Vector3f { 0, 0, 0 };

    // add vertices & indices to the renderer's default / small object "vbo"
    Entity *box = &cam;
    box->verts_low = vertex_buffer->len;
    box->lines_low = index_buffer->len;

    u16 vertex_offset = vertex_buffer->len;
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 1) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 2) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 3) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 0), (u16) (vertex_offset + 4) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 1), (u16) (vertex_offset + 2) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 2), (u16) (vertex_offset + 4) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 4), (u16) (vertex_offset + 3) };
    *(index_buffer->lst + index_buffer->len++) = Vector2_u16 { (u16) (vertex_offset + 3), (u16) (vertex_offset + 1) };

    *(vertex_buffer->lst + vertex_buffer->len++) = cam.origo;
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x - radius_xy, box->origo.y - radius_xy, box->origo.z + length_z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x - radius_xy, box->origo.y + radius_xy, box->origo.z + length_z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x + radius_xy, box->origo.y - radius_xy, box->origo.z + length_z };
    *(vertex_buffer->lst + vertex_buffer->len++) = Vector3f { box->origo.x + radius_xy, box->origo.y + radius_xy, box->origo.z + length_z };

    box->verts_high = vertex_buffer->len - 1;
    box->lines_high = index_buffer->len - 1;

    return cam;
}


//
// Point cloud-ish entities


Entity *EntityPoints(EntitySystem *es, Matrix4f transform, List<Vector3f> points) {
    Entity *pc = es->AllocEntityChain();
    pc->data_tpe = EF_EXTERNAL;
    pc->tpe = ET_POINTCLOUD;
    pc->entity_stream = NULL;
    pc->ext_points_lst = points;
    pc->ext_points = &pc->ext_points_lst;
    pc->color  = { RGBA_GREEN };
    pc->transform = transform;

    return pc;
}
Entity *EntityPoints(EntitySystem *es, List<Vector3f> points) {
    return EntityPoints(es, Matrix4f_Identity(), points);
}
Entity *EntityPointsNormals(EntitySystem *es) {
    Entity *pc = es->AllocEntityChain();
    pc->data_tpe = EF_EXTERNAL;
    pc->tpe = ET_POINTCLOUD_W_NORMALS;
    pc->entity_stream = NULL;
    pc->ext_points_lst = { NULL, 0 };
    pc->ext_points = &pc->ext_points_lst;
    pc->ext_normals_lst = { NULL, 0 };
    pc->ext_normals = &pc->ext_normals_lst;
    pc->color  = { RGBA_GREEN };
    pc->color_alt  = { RGBA_BLUE };
    pc->transform = Matrix4f_Identity();

    return pc;
}
Entity *EntityStream(EntitySystem *es, MArena *a_stream_bld, u32 npoints_max, u32 id, StreamHeader *prev, EntityType tpe, StreamType stpe) {
    StreamHeader *hdr = StreamReserveChain(a_stream_bld, npoints_max, Matrix4f_Identity(), prev, id, stpe);
    hdr->id = id;

    Entity *pc = es->AllocEntityChain();
    pc->data_tpe = EF_STREAM;
    pc->tpe = tpe;
    pc->entity_stream = hdr;
    pc->color  = { RGBA_GREEN };
    pc->transform = hdr->transform;

    return pc;
}
Entity *EntityStreamLoad(EntitySystem *es, StreamHeader *data, bool do_transpose) {
    Entity *ent = es->AllocEntityChain();
    ent->data_tpe = EF_STREAM;
    if (data->tpe == ST_POINTS) {
        // check if point cloud with normals:
        StreamHeader *data_nxt = data->GetNext(true);
        if (data_nxt != NULL && data_nxt->id == data->id && (data_nxt->tpe == ST_NORMALS || /*TODO: hax, remove:*/ data_nxt->tpe == ST_POINTS)) {
            ent->tpe = ET_POINTCLOUD_W_NORMALS;

            // assign ext data ptrs
            ent->ext_points_lst = data->GetDataVector3f();
            ent->ext_normals_lst = data_nxt->GetDataVector3f();
            ent->ext_points = &ent->ext_points_lst;
            ent->ext_normals = &ent->ext_normals_lst;

            assert(ent->ext_normals->len == ent->ext_points->len);
        }
        else {
            ent->tpe = ET_POINTCLOUD;
        }

        ent->color  = { RGBA_GREEN };
        if (do_transpose == true) {
            ent->transform = Matrix4f_Transpose( data->transform );
        }
        else {
            ent->transform = data->transform;
        }
    }
    // NOTE: load mesh entity stream here, if needed

    // assign stream ptr
    ent->entity_stream = data;
    if (ent->tpe == ET_POINTCLOUD_W_NORMALS) {
        assert((void*) ent->entity_stream->GetData() == (void*) ent->ext_points->lst && "extra consistence check for pc_w_normals");
    }

    return ent;
}


#endif
