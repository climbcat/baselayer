#ifndef __STREAM_H__
#define __STREAM_H__


#include "../baselayer.h"
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
#define RGBA_YELLOW     0, 255, 255, 255


#define BYTES_RGB 3
#define BYTES_RGBA 4


// TODO: change into Color4, Color3, etc.
struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};
void PrintColorInline(Color c) {
    printf("%hhx %hhx %hhx %hhx", c.r, c.g, c.b, c.a);
}


struct Rect {
    u16 width;
    u16 height;
    s16 left;
    s16 top;

    void Print() {
        printf("rect: w: %u, h: %u, left: %d, top: %d\n", width, height, left, top);
    }
};

Rect InitRectangle(u16 width, u16 height, u16 left = 0, u16 top = 0) {
    Rect r;
    r.width = width;
    r.height = height;
    r.left = left;
    r.top = top;
    return r;
}
Rect RectangleCrop(Rect us, Rect other) {
    Rect rect = InitRectangle(other.width, other.height, other.left, other.top);
    bool occluded = false;
    bool partially_occluded = false;

    // cases where other is completely outside of us
    if (other.left > us.left + us.width) { // to the right of us
        rect.left = us.left + us.width;
        rect.width = 0;
        occluded = true;
    }
    if (other.left + other.width < us.left) { // to the left of us
        rect.left = us.left;
        rect.width = 0;
        occluded = true;
    }
    if (other.top > us.top + us.height) { // above us
        rect.top = us.top + us.height;
        rect.height = 0;
        occluded = true;
    }
    if (other.top + other.height < us.top) { // below us
        rect.top = us.top;
        rect.height = 0;
        occluded = true;
    }
    if (occluded) {
        return rect;
    }

    // at least partially visible
    if (other.left < us.left) {
        rect.left = us.left;
        s16 diff = us.left - other.left;
        rect.width = rect.width - diff;
    }
    if (other.top < us.top) {
        rect.top = us.top;
        s16 diff = us.top - other.top;
        rect.height = rect.height - diff;
    }
    if (other.left + other.width > us.left + us.width) {
        rect.width = us.top + us.width;
    }
    if (other.top + other.height > us.top + us.height) {
        rect.height = us.top + us.height;
    }
    return rect;
}


//
// Data stream


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

struct ImageRGBX {
    u32 width;
    u32 height;
    u32 pixel_size;
    u8 *img;
};
ImageRGBX InitImageRGBX(void *data, u32 width, u32 height, u32 pixel_size) {
    ImageRGBX img;
    img.width = width;
    img.height = height;
    img.pixel_size = pixel_size;
    img.img = (u8*) data;
    return img;
}
struct ImageRGBA {
    u32 width;
    u32 height;
    Color *img;
};
struct ImageF32 {
    u32 width;
    u32 height;
    f32 *data;
};
struct ImageU32 {
    u32 width;
    u32 height;
    f32 *data;
};
struct StreamHeader {
    u32 next; // byte offset to the next StreamHeader location
    StreamType tpe;
    u32 id; // streams of the same id belong to one enity
    u32 time;
    u32 datasize;
    u32 linesize;
    Matrix4f transform;

    StreamHeader *GetNext(bool iter_nonext = false) {
        StreamHeader *result = NULL;
        if (iter_nonext && next == 0 && datasize > 0) {
            u32 calcnext = sizeof(StreamHeader) + datasize;
            result = (StreamHeader *) ((u8*) this + calcnext);
        }
        else if (next) {
            result = (StreamHeader *) ((u8*) this + next);
        }
        return result;
    }
    StreamHeader *GetNextTypeSameId(StreamType tpe) {
        StreamHeader *result = GetNext(true);
        while (result && result->tpe != tpe && result->id == id) {
            result = result->GetNext(true);
        }
        return result;
    }
    StreamHeader *GetNextId(bool iter_nonext = false) {
        StreamHeader *result = GetNext(iter_nonext);
        while (result && result->id == this->id) {
            result = result->GetNext(iter_nonext);
        }
        return result;
    }
    u8 *GetData() {
        return (u8*) this + sizeof(StreamHeader);
    }
    u32 GetDatumCount() {
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
        return linesize / sizeof(Vector3f);
    }
    u32 GetDatumSize() {
        return sizeof(Vector3f);
    }
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
StreamHeader *StreamCopy(MArena *a_stream_bld, Matrix4f transform, List<Vector3f> src, StreamType stpe, u32 id = 0, StreamHeader *prev = NULL) {
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

    // chain
    if (prev != NULL && result > prev) {
        prev->next = (u8*) result - (u8*) prev;
        assert((u8*) result == (u8*) prev + prev->next && "Stream chain contiguity violation");
    }

    return result;
}
StreamHeader *StreamReserve(MArena *a_stream_bld, Matrix4f transform, u32 npoints, u32 id = 0, StreamType stpe = ST_POINTS, StreamHeader *prev = NULL) {
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

    // chain
    if (prev != NULL) {
        assert(result > prev && "Stream data contiguity");
        prev->next = (u8*) result - (u8*) prev;
        assert((u8*) result == (u8*) prev + prev->next && "InitEntityStream data contiguity");
    }

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
