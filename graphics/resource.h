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


void ResourceStreamPushData(MArena *a_dest, ResourceStreamHandle *stream, ResourceType tpe, char *name, void *data, u32 data_sz) {
    assert(stream != NULL);

    stream->current = (ResourceHdr*) ArenaAlloc(a_dest, sizeof(ResourceHdr));
    stream->current->tpe = tpe;
    stream->current->data_sz = data_sz;
    _strcmp(stream->current->name, name);
    if (stream->prev) {
        stream->prev->next = (u32) ((u8*) stream->current - (u8*) stream->prev);
    }
    stream->prev = stream->current;
    if (stream->first == NULL) {
        stream->first = stream->current;
    }

    // push the data section
    ArenaPush(a_dest, data, data_sz);
}

void ResourceStreamPushDataExtra(MArena *a_dest, ResourceStreamHandle *stream, void *data, u32 data_sz) {
    ArenaPush(a_dest, data, data_sz);
    stream->current->data_sz += data_sz;
}

void ResourceStreamSave(ResourceStreamHandle *stream) {
    assert(stream->first != NULL);
    assert(stream->prev != NULL);

    void *data = stream->first;
    u32 last_sz = stream->prev->data_sz + sizeof(ResourceHdr);
    u32 data_sz = (u32) ((u8*) stream->prev - (u8*) stream->first + last_sz);

    SaveFile("all.res", data, data_sz);
}

// NOTE: this function is imagined to take up to one map arg per resource type in existence:
//      Currently, we have: fontatlas, byte texture. rgba-textures are expected, and perhaps sound resources
void ResourceStreamLoad(MArena *a_dest, u8 *resource_data, HashMap *map_fonts, HashMap *map_texture_bs) {
    assert(resource_data != NULL);

    ResourceHdr *resource = (ResourceHdr*) resource_data;

    s32 font_cnt = 0;
    while (resource) {
        if (resource->tpe == RT_FONT) {

            FontAtlas *font = FontAtlasLoadBinaryStream(resource->GetInlinedData(), resource->data_sz);
            
            MapPut(map_fonts, font->GetKey(), font);
            MapPut(map_texture_bs, font->GetKey(), &font->texture);
        }
        else {
            printf("WARN: non-font resource detected\n");
        }
        resource = resource->GetInlinedNext();
    }
    printf("loaded %d resources\n", font_cnt);
}


#endif
