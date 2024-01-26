#ifndef __ENTITYSYSTEM_H__
#define __ENTITYSYSTEM_H__


#include "../baselayer.h"
#include "stream.h"
#include "geometry.h"


//
// Entity System


enum EntityType {
    // lines etc.
    // TODO: eventually collapse into just ET_LINES
    ET_AXES,
    ET_LINES,
    ET_LINES_ROT,

    // data array
    ET_POINTCLOUD,
    ET_POINTCLOUD_W_NORMALS,
    ET_MESH,

    // UI related
    ET_BLITBOX_RGBA,
    ET_BLITBOX_RGB,

    // handle
    ET_EMPTY_NODE,

    // wildcard
    ET_ANY,

    // iteration len
    ET_CNT,
};
bool EntityIsTiedToRenderer(EntityType tpe) {
    if (tpe == ET_AXES || tpe == ET_LINES || tpe == ET_LINES_ROT) {
        return true;
    }
    else {
        return false;
    }
}
bool EntityHasVertices(EntityType tpe) {
    if (tpe == ET_POINTCLOUD || tpe == ET_POINTCLOUD_W_NORMALS || tpe == ET_MESH) {
        return true;
    }
    else {
        return false;
    }
}
bool EntityIsGeometrical(EntityType tpe) {
    if (tpe == ET_BLITBOX_RGB || tpe == ET_BLITBOX_RGBA || tpe == ET_EMPTY_NODE || tpe == ET_ANY || tpe == ET_CNT) {
        return false;
    }
    else {
        return true;
    }
}


//
// Entity


struct Entity {
    Entity *up = NULL;
    Entity *down = NULL;
    Entity *prev = NULL;
    Entity *next = NULL;

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
    ImageRGBA *ext_texture;
    ImageRGBA ext_texture_var;

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

    void SetVertexList(List<Vector3f> vertices) {
        ext_points_lst = vertices;
        ext_points = &ext_points_lst;
    }
    void SetNormalsList(List<Vector3f> normals) {
        ext_normals_lst = normals;
        ext_normals = &ext_normals_lst;
    }
    List<Vector3f> GetVertices() {
        assert((tpe == ET_POINTCLOUD || tpe == ET_MESH || tpe == ET_POINTCLOUD_W_NORMALS));

        List<Vector3f> verts { NULL, 0 };
        if (entity_stream != NULL) {
            verts = entity_stream->GetDataVector3f();
        }
        else if (ext_points != NULL) {
            verts = *ext_points;
        }
        else {
            verts = ext_points_lst;
        }
        return verts;
    }
    List<Vector3f> GetNormals() { 
        assert((tpe == ET_POINTCLOUD || tpe == ET_MESH || tpe == ET_POINTCLOUD_W_NORMALS));

        List<Vector3f> normals { NULL, 0 };
        if (entity_stream != NULL) {
            StreamHeader *stream = entity_stream->GetNextTypeSameId(ST_NORMALS);
            if (stream != NULL) {
                normals = stream->GetDataVector3f();
            }
        }
        else if (ext_normals != NULL) {
            normals = *ext_normals;
        }
        else {
            normals = ext_normals_lst;
        }
        return normals;
    }
    void SetStreamVertexCount(u32 npoints) {
        assert(tpe == ET_POINTCLOUD || tpe == ET_POINTCLOUD_W_NORMALS || tpe == ET_MESH);
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
        assert(tpe == ET_POINTCLOUD_W_NORMALS || tpe == ET_MESH);
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
    void SetTexture(ImageRGBA texture) {
        ext_texture_var = texture;
        ext_texture = &ext_texture_var;
    }
    void SetTexture(ImageRGBA *texture) {
        ext_texture_var.width = 0;
        ext_texture_var.height = 0;
        ext_texture_var.img = NULL;
        ext_texture = texture;
    }
    ImageRGBA GetTexture() {
        ImageRGBA img;
        if (ext_texture != NULL) {
            img = *ext_texture;
        }
        else {
            img = ext_texture_var;
        }
        return img;
    }
    void SetBlitBox(Rectangle rect) {
        dims.x = rect.width;
        dims.y = rect.height;
        dims.z = 0.0f;
        origo.x = rect.left;
        origo.y = rect.top;
        origo.z = 0.0f;
    }
    Rectangle GetBlitBox() {
        Rectangle rect = InitRectangle((u16) dims.x, (u16) dims.y, (s16) origo.x, (s16) origo.y);
        return rect;
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
void EntityAppendChild(Entity *ent_new, Entity *branch) {
    // append an entity below-after a certain branch, meaning below of after a chain starting at branch.down
    assert(branch != NULL);

    EntityYank(ent_new);
    ent_new->up = branch;
    if (branch->down == NULL) {
        branch->down = ent_new;
        ent_new->up = branch;
    }
    else {
        Entity *nxt = branch->down;
        while (nxt->next != NULL) {
            nxt = nxt->next;
        }
        nxt->next = ent_new;
        ent_new->prev = nxt;
    }
}


//
// Organization of entities


#define TREE_ITER_STACK_CAPACITY 200
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
    iter->_stc.len = 0;
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
    u32 FreeEntityBranch(Entity *branch_root) {
        u32 cnt;
        if (branch_root) {
            Entity *ent = branch_root;
            Entity *next;
            TreeIterState itr;
            InitTreeIter(&itr);
            while (ent) {
                next = TreeIterNext(ent, &itr, false);
                FreeEntity(ent);
                cnt++;
                ent = next;
            }
        }
        return cnt;
    }
    u32 FreeEntitiesBelowBranch(Entity *branch_root) {
        u32 cnt = 0;
        if (branch_root->down != NULL) {
            FreeEntityBranch(branch_root->down);
        }
        return cnt;
    }
    Entity *AllocEntityChild(Entity *parent = NULL) {
        Entity default_val;
        Entity *next = (Entity*) PoolAlloc(&pool);
        assert(next != NULL && "pool capacity exceeded");
        *next = default_val;

        if (parent == NULL) {
            EntityAppendChild(next, this->root);
        }
        else {
            EntityAppendChild(next, parent);
        }
        return next;
    }


    //
    // Cursor


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
        if (EntityIsTiedToRenderer(next->tpe)) {
            printf("%u: analytic,   vertices %u -> %u lines %u -> %u\n", eidx, next->verts_low, next->verts_high, next->lines_low, next->lines_high);
        }
        else if (EntityHasVertices(next->tpe)) {
            printf("%u: data, %lu bytes\n", eidx, next->GetVertices().len * sizeof(Vector3f));
        }
        else {
            printf("%u: other\n", eidx);
        }
        if (EntityIsGeometrical(next->tpe)) {
            PrintTransform(next->transform);
        }
        eidx++;
        next = es->TreeIterNext(next, &iter, false);
    }
}


static EntitySystem _g_entity_system;
static EntitySystem *g_entity_system;
EntitySystem *InitEntitySystem() {
    if (g_entity_system != NULL) {
        return g_entity_system;
    }
    g_entity_system = &_g_entity_system;

    g_entity_system->pool = PoolCreate(sizeof(Entity), ENTITY_MAX_CNT);
    g_entity_system->cursor_highlight_color = Color { RGBA_WHITE };
    // TODO: validate this great idea -->
    void *zero = PoolAlloc(&g_entity_system->pool);

    g_entity_system->root = (Entity*) PoolAlloc(&g_entity_system->pool);
    *g_entity_system->root = InitEntity(ET_EMPTY_NODE);
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


Entity *EntityBranchHandle(EntitySystem *es, Entity* branch) {
    Entity *ent = es->AllocEntityChild(branch);
    ent->tpe = ET_EMPTY_NODE;
    return ent;
}
Entity *EntityPoints(EntitySystem *es, Entity* branch, Matrix4f transform, List<Vector3f> points) {
    Entity *pc = es->AllocEntityChild(branch);
    pc->tpe = ET_POINTCLOUD;
    pc->entity_stream = NULL;
    pc->ext_points_lst = points;
    pc->ext_points = &pc->ext_points_lst;
    pc->color  = { RGBA_GREEN };
    pc->transform = transform;

    return pc;
}
Entity *EntityPoints(EntitySystem *es, Entity* branch, List<Vector3f> points) {
    return EntityPoints(es, branch, Matrix4f_Identity(), points);
}
Entity *EntityPointsNormals(EntitySystem *es, Entity* branch) {
    Entity *pc = es->AllocEntityChild(branch);
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
Entity *EntityMesh(EntitySystem *es, Entity* branch) {
    Entity *pc = es->AllocEntityChild(branch);
    pc->tpe = ET_MESH;
    pc->entity_stream = NULL;
    pc->ext_points_lst = { NULL, 0 };
    pc->ext_points = &pc->ext_points_lst;
    pc->ext_normals_lst = { NULL, 0 };
    pc->ext_normals = &pc->ext_normals_lst;
    pc->color  = { RGBA_RED };
    pc->color_alt  = { RGBA_GRAY_25 };
    pc->transform = Matrix4f_Identity();

    return pc;
}
Entity *EntityStream(EntitySystem *es, Entity* branch, MArena *a_stream_bld, u32 npoints_max, u32 id, StreamHeader *prev, EntityType tpe, StreamType stpe) {
    StreamHeader *hdr = StreamReserveChain(a_stream_bld, npoints_max, Matrix4f_Identity(), prev, id, stpe);
    hdr->id = id;

    Entity *pc = es->AllocEntityChild(branch);
    pc->tpe = tpe;
    pc->entity_stream = hdr;
    pc->color  = { RGBA_GREEN };
    pc->transform = hdr->transform;

    return pc;
}
Entity *EntityStreamLoad(EntitySystem *es, Entity* branch, StreamHeader *data, bool do_transpose) {
    Entity *ent = es->AllocEntityChild(branch);
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


//
// 2d / UI element / texture entities


Entity *EntityBlitBox(EntitySystem *es, Entity* branch, Rectangle box, ImageRGBA src) {
    Entity *bb = es->AllocEntityChild(branch);
    bb->tpe = ET_BLITBOX_RGB;
    bb->SetTexture(src);
    bb->SetBlitBox(box);

    // TODO: use zero-is-initialization on all entities
    bb->entity_stream = NULL;
    bb->ext_points_lst = { NULL, 0 };
    bb->ext_points = &bb->ext_points_lst;
    bb->ext_normals_lst = { NULL, 0 };
    bb->ext_normals = &bb->ext_normals_lst;
    bb->color  = { RGBA_WHITE };
    bb->color_alt  = { RGBA_GRAY_25 };
    bb->transform = Matrix4f_Identity();
    return bb;
}


#endif
