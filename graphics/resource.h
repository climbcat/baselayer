#ifndef __RESOURCE_H__
#define __RESOURCE_H__


#include "../baselayer.h"
#include "atlas.h"


enum ResourceType {
    RT_FONT,
    RT_SPRITE,

    RT_CNT,

    RT_RESOURCE_CNT,
    RT_RESOURCE_DATA,
    RT_RESOURCE_NAME
};


struct HexResourceHdr {
    ResourceType tpe;
    u32 size;
    const char *name;
};



GlyphPlotter *LoadResource(MArena *a_dest, HexResourceHdr *resource) {
    assert(resource != NULL);

    HexResourceHdr *res = resource;
    HexResourceHdr *entries = resource + 2;

    // resource headers count
    assert(res->tpe == RT_RESOURCE_CNT);
    u32 cnt = res->size;
    res++;

    // resource name as a string
    assert(res->tpe == RT_RESOURCE_NAME);
    const char *res_name = res->name;
    res++;

    // resource headers
    u32 size_accum = 0;
    for (u32 i = 0; i < cnt; ++i) {
        size_accum += res[i].size;
    }

    // resource raw data
    assert(res[cnt].tpe == RT_RESOURCE_DATA);
    assert(res[cnt].size == 2 * size_accum); // there are two chars per hex-encoded byte
    char *data = (char*) res[cnt].name;


    // TODO: create the map of objects
    

    // set map entries
    for (u32 j = 0; j < cnt; ++j) {

        // TODO: init and register the object into a map
        if (entries[j].tpe == RT_FONT) {
            u32 size = entries[j].size;
            u8* data_dest = (u8*) ArenaAlloc(a_dest, size);

            const char *nibble_to_hex = "0123456789ABCDEF";

            {
                TimeBlock("hex_parse");
                
                for (u32 i = 0; i < size; ++i) {
                    // TODO: replace with impl. at the second anaswer at s.o. 3408706, which is homebrew and indeed 10x faster
                    sscanf(data + 2*i, "%2hhx", data_dest + i);

                    /*
                    // DEBUG print:
                    printf("%c%c", *(data + 2*i), *(data + 2*i + 1));
                    u8 byte = data_dest[i];
                    char a = nibble_to_hex[byte >> 4];
                    char b = nibble_to_hex[byte & 0x0F];
                    printf(" --> %c%c\n", a, b);
                    */
                }
            }


            FontAtlas *font = FontAtlasLoadBinaryStream(data_dest, size);
            GlyphPlotter * plt = InitGlyphPlotter(a_dest, font);

            printf("loaded a plotter ! \n");

            GlyphPlotterPrint(plt);

            return plt;
            // TODO: enter the plotter thing into the thing
        }

        // inc the object pointer
    }
}


#endif
