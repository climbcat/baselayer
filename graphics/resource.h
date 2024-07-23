#ifndef __RESOURCE_H__
#define __RESOURCE_H__


#include "../baselayer.h"


enum ResourceType {
    RT_FONT,
    RT_SPRITE,

    RT_CNT
};
void PrintResourceType(ResourceType tpe) {
    if (tpe == RT_FONT) {
        printf("font\n");
    }
    else if (tpe == RT_SPRITE) {
        printf("sprite\n");
    }
    else {
        printf("_unknown_\n");
    }
}


struct ResourceHdr {
    ResourceType tpe;
    u32 data_sz;
    u32 next;
    char key_name[200];

    ResourceHdr *GetInlinedNext() {
        if (next == 0) {
            return NULL;
        }
        ResourceHdr *nxt =  (ResourceHdr*) ((u8*) this + next);
        return nxt;
    }
    u8 *GetInlinedData() {
        u8 *dta =  (u8*) this + sizeof(ResourceHdr);
        assert( (dta + data_sz) == (u8*) GetInlinedNext() || GetInlinedNext() == NULL );

        return dta;
    }
};


struct ResourceStreamHandle {
    ResourceHdr *first;
    ResourceHdr *prev;
    ResourceHdr *current;
};


ResourceStreamHandle ResourceStreamLoadAndOpen(MArena *a_dest, const char *filename) {    
    ResourceStreamHandle hdl = {};
    hdl.first = (ResourceHdr *) LoadFileFSeek(a_dest, (char*) filename);

    ResourceHdr *res = hdl.first;
    while (res) {
        hdl.prev = res;

        PrintResourceType(res->tpe);

        res = res->GetInlinedNext();
    }

    return hdl;
}


void ResourceStreamPushData(MArena *a_dest, ResourceStreamHandle *stream, ResourceType tpe, char *key_name, void *data, u32 data_sz) {
    assert(stream != NULL);

    stream->current = (ResourceHdr*) ArenaAlloc(a_dest, sizeof(ResourceHdr));
    stream->current->tpe = tpe;
    stream->current->data_sz = data_sz;
    _strcmp(stream->current->key_name, key_name);
    if (stream->prev) {
        stream->prev->next = (u32) ((u8*) stream->current - (u8*) stream->prev);
    }
    stream->prev = stream->current;
    if (stream->first == NULL) {
        stream->first = stream->current;
    }
    ArenaPush(a_dest, data, data_sz);
}
void ResourceStreamPushData(MArena *a_dest, ResourceStreamHandle *stream, ResourceType tpe, const char *key_name, void *data, u32 data_sz) {
    return ResourceStreamPushData(a_dest, stream, tpe, (char*) key_name, data, data_sz);
}

void ResourceStreamPushDataExtra(MArena *a_dest, ResourceStreamHandle *stream, void *data, u32 data_sz) {
    ArenaPush(a_dest, data, data_sz);
    stream->current->data_sz += data_sz;
}

void ResourceStreamSave(ResourceStreamHandle *stream, const char *filename = "all.res") {
    assert(stream->first != NULL);
    assert(stream->prev != NULL);

    void *data = stream->first;
    u32 last_sz = stream->prev->data_sz + sizeof(ResourceHdr);
    u32 data_sz = (u32) ((u8*) stream->prev - (u8*) stream->first + last_sz);

    SaveFile(filename, data, data_sz);
}


#endif
