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
        printf("sprite map\n");
    }
    else {
        printf("_unknown_\n");
    }
}


struct ResourceHdr {
    ResourceType tpe;
    u32 data_sz;
    u32 next;
    char name[64];
    char key_name[64];

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
    u32 cnt;
    u32 cnt_tpe[RT_CNT];
    HashMap map_names;
    HashMap map_keynames;
    StrLst *names;
    StrLst *key_names;

    ResourceHdr *first;
    ResourceHdr *prev;
    ResourceHdr *current;
};


// TODO: have the total resource count in the ResourceStreamHandle
// TODO: ensure global key_name uniqueness on some level (?)
// TODO: enable a key-iteration option in the HashMap helpers


ResourceStreamHandle ResourceStreamLoadAndOpen(MArena *a_tmp, MArena *a_dest, const char *filename) {    
    ResourceStreamHandle hdl = {};
    hdl.first = (ResourceHdr *) LoadFileFSeek(a_dest, (char*) filename);
    hdl.map_names = InitMap(a_tmp, 1000); // TODO: how do we put a bound on the number of resources in a file??
    hdl.map_keynames = InitMap(a_tmp, 1000); // TODO: how do we put a bound on the number of resources in a file??

    StrLst *names = NULL;
    StrLst *key_names = NULL;
    ResourceHdr *res = hdl.first;
    while (res) {
        hdl.prev = res;
        hdl.cnt++;
        hdl.cnt_tpe[res->tpe]++;

        // check keyname uniqueness & record unique names and keynames
        u64 key = HashStringValue(res->key_name);
        if (MapGet(&hdl.map_keynames, key)) {
            assert( MapGet(&hdl.map_keynames, key) == 0 && "resource key duplicate");
        }
        key_names = StrLstPut(a_dest, res->key_name, key_names);
        MapPut(&hdl.map_names, key, res);
        key = HashStringValue(res->name);
        if (MapGet(&hdl.map_names, key) == 0) {
            MapPut(&hdl.map_names, key, res);
            names = StrLstPut(a_dest, res->name, names);
        }

        // iter
        res = res->GetInlinedNext();
    }
    printf("opened resource file '%s': %u entries (", filename, hdl.cnt);
    for (u32 i = 0; i < RT_CNT; ++i) {
        printf("%u", hdl.cnt_tpe[i]);
        if (i + 1 < RT_CNT) {
            printf(", ");
        }
    }
    printf(")\n");

    hdl.key_names = key_names->first;
    hdl.names = names->first;

    return hdl;
}


void ResourceStreamPushData(MArena *a_dest, ResourceStreamHandle *stream, ResourceType tpe, char *name, char *key_name, void *data, u32 data_sz) {
    assert(stream != NULL);

    stream->current = (ResourceHdr*) ArenaAlloc(a_dest, sizeof(ResourceHdr));
    stream->current->tpe = tpe;
    stream->current->data_sz = data_sz;
    _memcpy(stream->current->name, key_name, _strlen(name));
    _memcpy(stream->current->key_name, key_name, _strlen(key_name));
    if (stream->prev) {
        stream->prev->next = (u32) ((u8*) stream->current - (u8*) stream->prev);
    }
    stream->prev = stream->current;
    if (stream->first == NULL) {
        stream->first = stream->current;
    }
    ArenaPush(a_dest, data, data_sz);
}

void ResourceStreamPushData(MArena *a_dest, ResourceStreamHandle *stream, ResourceType tpe, char *name, const char *key_name, void *data, u32 data_sz) {
    return ResourceStreamPushData(a_dest, stream, tpe, (char*) name, (char*) key_name, data, data_sz);
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
