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
Vector3f _PointCloudAverage(Vector3f *data, u32 npoints, bool nonzero_only) {
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
        Vector3f av = _PointCloudAverage((Vector3f*) et->GetData(), et->GetDatumCount(), true);
        printf(" av. %f %f %f\n", av.x, av.y, av.z);
        et = et->GetNext();
        ++didx;
    }
}
StreamHeader *StreamCopy(MArena *a_stream_bld, Matrix4f transform, List<Vector3f> src, u32 id = 0, StreamType stpe = ST_POINTS) {
    // allocate and copy header + payload
    StreamHeader hdr;
    hdr.next = 0;
    hdr.tpe = stpe;
    hdr.id = id;
    hdr.transform = transform;
    hdr.SetVertexCount(src.len); // sets .datasize
    hdr.time = 0;
    hdr.linesize = 0;

    StreamHeader *result = (StreamHeader*) ArenaPush(a_stream_bld, &hdr, sizeof(StreamHeader));
    ArenaPush(a_stream_bld, src.lst, hdr.datasize);

    return result;
}
StreamHeader *StreamReserve(MArena *a_stream_bld, Matrix4f transform, u32 npoints, u32 id = 0, StreamType stpe = ST_POINTS) {
    // allocate and copy header, allocate payload (no copy)
    StreamHeader hdr;
    hdr.next = 0;
    hdr.tpe = stpe;
    hdr.id = id;
    hdr.transform = transform;
    hdr.SetVertexCount(npoints); // sets .datasize
    hdr.time = 0;
    hdr.linesize = 0;

    StreamHeader *result = (StreamHeader*) ArenaPush(a_stream_bld, &hdr, sizeof(StreamHeader));
    ArenaAlloc(a_stream_bld, hdr.datasize);

    return result;
}
StreamHeader *StreamReserveChain(MArena *a_stream_bld, u32 npoints, Matrix4f transform, StreamHeader *prev = NULL, u32 id = 0, StreamType stpe = ST_POINTS) {
    StreamHeader *hdr = StreamReserve(a_stream_bld, transform, npoints, id, stpe);

    // link here from prev
    if (prev != NULL) {
        assert(hdr > prev && "InitEntityStream data contiguity");
        prev->next = (u8*) hdr - (u8*) prev;
        assert((u8*) hdr == (u8*) prev + prev->next && "InitEntityStream data contiguity");
    }

    return hdr;
}
void StreamTrimTail(MArena *a_stream_bld, u32 npoints_actual, StreamHeader *hdr) {
    // Use when [ npoints_actual < npoints_max ] to shorten the stream tail. Usually happens during data capture or filtering
    u32 bytes_actual = npoints_actual * hdr->GetDatumSize();
    u32 bytes_reserved = hdr->datasize;

    assert(bytes_actual <= bytes_reserved && "EntityStreamFinalize: bytes_actual <= bytes_reserved");
    assert(a_stream_bld->mem <= (u8*) hdr && "EntityStreamFinalize: header >= arena base pointer");
    assert(a_stream_bld->mem + a_stream_bld->used == (u8*) hdr->GetData() + bytes_reserved && "EntityStreamFinalize: reserved bytes == current arena pointer");

    a_stream_bld->used = a_stream_bld->used - (bytes_reserved - bytes_actual);
    hdr->datasize = bytes_actual;
}


#endif
