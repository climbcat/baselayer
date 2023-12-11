#ifndef __STREAM_H__
#define __STREAM_H__


#include "../baselayer.h"
#include "geometry.h"


//
// Entity System


enum StreamType {
    ST_POINTS,
    ST_VERTICES,
    ST_NORMALS,
    ST_INDICES2,
    ST_INDICES3,
    ST_RGBA, // bitmap (4B stride)
    ST_F32, // depth image (4B stride)
    ST_TEXCOORDS, // [0;1]^2 image, (8B stride)

    ST_CNT,
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
struct StreamHeader {
    u32 next; // purpose: byte offset for iterating the chunk, zero indicates the final entry. (!Can in principle be != sizeof(EntityStream) + datasize)
    StreamType tpe; // purpose: allows interpretation of data payload
    u32 id; // purpose: for associating different EntityStream entries with the same object within the chunk (e.g. depth + colour)
    u32 time; // purpose: real-time data header info for sorting & filtering by post- or external process
    u32 datasize; // payload size
    u32 linesize; // line size for 2d data
    Matrix4f transform; // purpose: storage of this ever-present header info

    //
    // utility functions

    StreamHeader *GetNext(bool load_nonext = false) {
        StreamHeader *result = NULL;
        if (load_nonext && next == 0 && datasize > 0) {
            u32 calcnext = sizeof(StreamHeader) + datasize;
            result = (StreamHeader *) ((u8*) this + calcnext);
        }
        else if (next) {
            result = (StreamHeader *) ((u8*) this + next);
        }
        return result;
    }
    StreamHeader *GetNextId(bool load_nonext = false) {
        StreamHeader *result = GetNext(load_nonext);
        while (result && result->id == this->id) {
            result = result->GetNext(load_nonext);
        }
        return result;
    }
    u8 *GetData() {
        // returns a pointer to where the data is supposed to start (right after this struct's location in memory)
        return (u8*) this + sizeof(StreamHeader);
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
void StreamPrint(StreamHeader *et, char *tag) {
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
StreamHeader *InitStream(MArena *a, StreamType tpe, u32 npoints_max, StreamHeader *prev, u32 id = 0) {
    // NOTE: next is safely set on prev here
    StreamHeader *es = (StreamHeader*) ArenaAlloc(a, sizeof(StreamHeader), true);
    es->next = 0;
    es->id = id;
    es->time = 0;
    es->SetVertexCount(npoints_max);
    es->linesize = 0;
    es->transform = Matrix4f_Identity();
    es->tpe = tpe;

    // linked list pointer is assigned on prevíous element
    if (prev != NULL) {
        assert(es > prev && "InitEntityStream data contiguity");
        prev->next = (u8*) es - (u8*) prev;
        assert((u8*) prev + prev->next == (u8*) es && "InitEntityStream data contiguity");
    }

    // TODO: switch by data type to determine allocation size

    // allocate / reserve npoints worth of memory
    List<Vector3f> points = InitList<Vector3f>(a, npoints_max);
    assert( (Vector3f*) points.lst == (Vector3f*) es->GetData() && "InitEntityStream" );

    return es;
}
u32 PointerDiff(void *from, void *to) {
    assert(from < to && "PointerDiff");
    u32 result = (u8*) to - (u8*) from;
    return result;
}
u8 *PointerOffsetByDist(void *ptr, int offset_dist) {
    assert(ptr != NULL && "PointerOffsetByDist");
    u8 *result = (u8*) ptr + offset_dist;
    return result;
}
void InitStreamFinalize(MArena *a, u32 npoints_actual, StreamHeader *hdr) {
    // Use with Inittream after knowning that npoints_actual < npoints_max
    // NOTE: next is set on "prev_hdr" during Init
    u32 bytes_actual = npoints_actual * hdr->GetDatumSize();
    u32 bytes_reserved = hdr->datasize;

    assert(bytes_actual <= bytes_reserved && "EntityStreamFinalize: bytes_actual <= bytes_reserved");
    assert(a->mem <= (u8*) hdr && "EntityStreamFinalize: header >= arena base pointer");
    assert(a->mem + a->used == (u8*) hdr->GetData() + bytes_reserved && "EntityStreamFinalize: reserved bytes == current arena pointer");

    a->used = a->used - (bytes_reserved - bytes_actual);
    hdr->datasize = bytes_actual;
}
StreamHeader *StreamCopy(MArena *a, Matrix4f transform, List<Vector3f> src, u32 id) {
    // allocate and copy header + payload
    StreamHeader stream;
    stream.next = 0;
    stream.tpe = ST_POINTS;
    stream.id = id;
    stream.transform = transform;
    stream.SetVertexCount(src.len);

    StreamHeader *result = (StreamHeader*) ArenaPush(a, &stream, sizeof(stream));
    ArenaPush(a, src.lst, stream.datasize);

    return result;
}
StreamHeader *StreamReserve(MArena *a, Matrix4f transform, List<Vector3f> src, u32 id) {
    // allocate and copy header, allocate payload (no copy)
    StreamHeader stream;
    stream.next = 0;
    stream.tpe = ST_POINTS;
    stream.id = id;
    stream.transform = transform;
    stream.SetVertexCount(src.len);

    StreamHeader *result = (StreamHeader*) ArenaPush(a, &stream, sizeof(stream));
    ArenaAlloc(a, stream.datasize);

    return result;
}


#endif
