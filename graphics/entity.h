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
    ET_MESH,

    ET_CNT,
};
enum EntityTypeFilter {
    EF_ANALYTIC,
    EF_EXTERNAL,
    EF_ANY,

    EF_CNT,
};


//
// Entity


struct Entity {
    // header
    u16 up = 0;
    u16 down = 0;
    u16 prev = 0;
    u16 next = 0;
    EntityTypeFilter data_tpe;
    EntityType tpe;
    Matrix4f transform; // TODO: make this into a transform_id and store externally
    Color color { RGBA_WHITE };

    // analytic entity indices (kept in a single "vbo" within the renderer)
    u16 verts_low = 0;
    u16 verts_high = 0;
    u16 lines_low = 0;
    u16 lines_high = 0;

    // utility fields
    Vector3f origo;
    Vector3f dims;

    // external data
    StreamHeader *entity_stream;

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

    // data entity parameters (pointcloud, ...)
    List<u8> GetData() {
        List<u8> data;
        if (entity_stream != NULL) {
            data.lst = entity_stream->GetData();
            data.len = entity_stream->datasize;
        }
        return data;
    }
    List<Vector3f> GetVertices() {

        if (!(data_tpe == EF_EXTERNAL && (tpe == ET_POINTCLOUD || tpe == ET_MESH))) {
            printf("her\n");
        }

        assert(data_tpe == EF_EXTERNAL && (tpe == ET_POINTCLOUD || tpe == ET_MESH) && "GetVertices: Only call with point cloud or mesh ext data");
        List<Vector3f> verts;
        if (entity_stream != NULL) {
            verts = entity_stream->GetDataVector3f();
        }
        return verts;
    }
    List<Vector3i> GetIndices() {
        List<Vector3i> indices;
        StreamHeader *nxt = this->entity_stream;
        if (entity_stream != NULL) {
            while (nxt->tpe != ST_INDICES3) {
                nxt = nxt->GetNext();
            }
            indices = entity_stream->GetDataVector3i();
        }
        return indices;
    }
    void SetVertexCount(u32 npoints) {
        assert(data_tpe == EF_EXTERNAL && (tpe == ET_POINTCLOUD || tpe == ET_MESH));
        entity_stream->SetVertexCount(npoints);
    }

    // scene graph behavior
    void Activate() {
        active = true;
    }
    void DeActivate() {
        active = false;
    }
};
Entity InitEntity(EntityType tpe) {
    Entity e;
    e.tpe = tpe;
    if (tpe == ET_MESH || tpe == ET_POINTCLOUD) {
        e.data_tpe = EF_EXTERNAL;
    }
    else {
        e.data_tpe = EF_ANALYTIC;
    }
    e.transform = Matrix4f_Identity();
    e.active = true;
    return e;
}


//
// Organization of entities


#define ENTITY_MAX_COUNT 200
struct EntitySystem {
    MPool pool;
    u16 first = 0;
    u16 iter_next = 0;
    u16 last = 0;

    Entity *GetEntityByIdx(u16 idx) {
        Entity *result = (Entity*) PoolIdx2Ptr(&pool, idx);
        return result;
    }
    void IterReset() {
        iter_next = first;
    }
    Entity *IterNext(EntityTypeFilter data_tpe = EF_ANY) {
        Entity *result = GetEntityByIdx(iter_next);
        if (result == NULL) {
            iter_next = 0;
            return NULL;
        }
        iter_next = result->next;
        if (data_tpe == EF_ANY || result->data_tpe == data_tpe) {
            return result;
        }
        else {
            return IterNext(data_tpe);
        }
    }
    Entity *IterNext(Entity *prev, EntityTypeFilter data_tpe = EF_ANY) {
        if (prev == NULL) {
            prev = GetEntityByIdx(first);
        }
        Entity *result = GetEntityByIdx(prev->next);
        if (result == NULL) {
            return NULL;
        }
        if (data_tpe == EF_ANY || result->data_tpe == data_tpe) {
            return result;
        }
        else {
            return IterNext(result, data_tpe);
        }
    }
    Entity *AllocEntity() {
        Entity default_val;
        Entity *result = (Entity*) PoolAlloc(&pool);
        *result = default_val;
        return result;
    }
};
EntitySystem InitEntitySystem() {
    EntitySystem es;
    es.pool = PoolCreate(sizeof(Entity), ENTITY_MAX_COUNT);
    void *zero = PoolAlloc(&es.pool);
    return es;
}
void EntitySystemChain(EntitySystem *es, Entity *next) {
    u16 next_idx = PoolPtr2Idx(&es->pool, next);
    if (es->first == 0) {
        es->first = next_idx;
        es->last = next_idx;
        es->IterReset();
    }
    else {
        Entity * es_last = es->GetEntityByIdx(es->last);
        es_last->next = next_idx;
        es->last = next_idx;
    }
}
void EntitySystemPrint(EntitySystem *es) {
    u32 eidx = 0;
    printf("entities: \n");
    es->IterReset();
    Entity *next = es->IterNext();
    while (next != NULL) {
        if (next->data_tpe == EF_ANALYTIC) {
            printf("%u: analytic, vertices %u -> %u lines %u -> %u\n", eidx, next->verts_low, next->verts_high, next->lines_low, next->lines_high);
        }
        else {
            printf("%u: data#%d, %u bytes\n", eidx, next->entity_stream->tpe, next->entity_stream->datasize);
        }
        eidx++;
        next = es->IterNext();
    }
}


#endif
