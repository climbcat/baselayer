#ifndef __RESOURCE_H__
#define __RESOURCE_H__


#include "../baselayer.h"
#include "atlas.h"


enum ResourceType {
    RT_FONT,
    RT_SPRITE,

    RT_CNT
};


struct ResourceHdr {
    ResourceType tpe;
    u32 data_sz;
    u32 next;
    char name[200];
};


struct ResourceStreamHandle {
    ResourceHdr *first;
    ResourceHdr *last;
};


void ResourceStreamPushData(MArena *a_dest, ResourceStreamHandle *stream, ResourceType tpe, char *name, void *data, u32 data_sz) {
    assert(stream != NULL);

    ResourceHdr *hdr = (ResourceHdr*) ArenaAlloc(a_dest, sizeof(ResourceHdr));
    hdr->tpe = tpe;
    hdr->data_sz = data_sz;
    _strcmp(hdr->name, name);
    if (stream->last) {
        stream->last->next = (u32) ((u8*) stream->last - (u8*) hdr);
    }
    stream->last = hdr;
    if (stream->first == NULL) {
        stream->first = hdr;
    }

    // push the data section
    ArenaPush(a_dest, data, data_sz);
}


void ResourceStreamSave(ResourceStreamHandle *stream) {
    assert(stream->first != NULL);
    assert(stream->last != NULL);

    void *data = stream->first;
    u32 last_sz = stream->last->data_sz + sizeof(ResourceHdr);
    u32 data_sz = (u32) ((u8*) stream->last - (u8*) stream->first + last_sz);

    SaveFile("all.res", data, data_sz);
}


void ResourceStreamLoad(MArena *a_dest, ResourceHdr *resource) {
    // TODO: try and have only inlined data, e.g. no areana needed to expand / unpack anything during initialization
    //      (FontAtlas I am looking at u).
    assert(resource != NULL);

    // TODO: init every data object (inline !)
    // TODO: put every data object type in its own map
    // TODO: don't over do this, we don't have any sprites as of yet
}


#endif
