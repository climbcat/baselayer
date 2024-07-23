#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__


#include <math.h>
#include <assert.h>

#include "gtypes.h"
#include "entity.h"
#include "geometry.h"

#include "ui.h"

#include "resource.h"
#include "sprite.h"
#include "font.h"

#include "imui.h"

#include "shaders.h"
#include "swrender.h"
#include "gameloop.h"



// TODO: Resource - Font and Sprite - initialization goes exactly where?


// TODO: size according to resource.h given resource count
#define ATLAS_MAX_CNT 100


void LoadFontsFromResourceStream(ResourceHdr *resource, HashMap *map_fonts, HashMap *map_texture_bs) {
    s32 font_cnt = 0;
    while (resource) {
        if (resource->tpe == RT_FONT) {

            FontAtlas *font = FontAtlasLoadBinaryStream(resource->GetInlinedData(), resource->data_sz);
            
            MapPut(map_fonts, font->GetKey(), font);
            MapPut(map_texture_bs, font->GetKey(), &font->texture);

            font_cnt++;
        }
        else {
            printf("WARN: non-font resource detected\n");
        }
        resource = resource->GetInlinedNext();
    }
    printf("loaded %d resources\n", font_cnt);
}


void InitFonts(MContext *ctx) {
    assert(g_texture_map.slots.len != 0 && "check sprites were initialized");
    if (g_font_map.slots.len != 0) {
        printf("WARN: re-init fonts\n");
        return;
    }

    g_font_map = InitMap(ctx->a_life, ATLAS_MAX_CNT);

    u64 sz_file;
    u8 *readonly_data = (u8*) LoadFileMMAP("all.res", &sz_file);
    u8 *resource_data = (u8*) ArenaPush(ctx->a_life, readonly_data, sz_file);

    if (resource_data == NULL) {
        printf("please supply an 'all.res' font resource file with the executable, exiting ...\n");
        exit(0);
    }
    LoadFontsFromResourceStream((ResourceHdr*) resource_data, &g_font_map, &g_texture_map);


    // TODO: semantic compression


    List<u64> fn_hashes = InitList<u64>(ctx->a_tmp, ATLAS_MAX_CNT);
    StrLst *font_names_iter = NULL;

    // iterate through all keys (put this into the hash.h file)
    for (u32 i = 0; i < g_font_map.slots.len; ++i) {
        HashMapKeyVal kv = g_font_map.slots.lst[i];
        if (kv.key != 0) {
            FontAtlas *atlas = (FontAtlas*) g_font_map.slots.lst[i].val;

            if (fn_hashes.AddUnique(HashStringValue(atlas->font_name)) != NULL) {
                Str entry = { atlas->font_name, _strlen(atlas->font_name) };
                font_names_iter = StrLstPut(entry, font_names_iter);
                if (g_font_names == NULL) {
                    g_font_names = font_names_iter;
                }
            }
        }
    }
    for (u32 i = 0; i < g_font_map.colls.len; ++i) {
        HashMapKeyVal kv = g_font_map.colls.lst[i];
        if (kv.key != 0) {
            FontAtlas *atlas = (FontAtlas*) g_font_map.colls.lst[i].val;

            if (fn_hashes.AddUnique(HashStringValue(atlas->font_name)) != NULL) {
                Str entry = { atlas->font_name, _strlen(atlas->font_name) };
                font_names_iter = StrLstPut(entry, font_names_iter);
                if (g_font_names == NULL) {
                    g_font_names = font_names_iter;
                }
            }
        }
    }

    printf("loaded %d fonts: \n", StrListLen(g_font_names));
    StrLstPrint(g_font_names);


    //


    SetFontAndSize(FS_48, g_font_names->GetStr());
}



void __PrintResourceType(ResourceType tpe, void *data, u32 data_size) {
    if (tpe == RT_FONT) {
        printf("font\n");
    }
    else if (tpe == RT_SPRITE) {
        // TODO: move down into dependant
        SpriteMap *smap = SpriteMapLoadStream((u8*) data, data_size);
        printf("sprite map: %s, %s, count: %u, atlas w: %u, atlas h: %u\n", smap->map_name, smap->key_name, smap->sprites.len, smap->texture.width, smap->texture.height);

        /*
        List<Sprite> ss = smap->sprites;
        for (u32 i = 0; i < ss.len; ++i) {
            Sprite s = ss.lst[i];
            PrintSprite(s);
        }
        */
    }
    else {
        printf("_unknown_\n");
    }
}


GameLoopOne *InitGraphics(MContext *ctx) {
    GameLoopOne *loop = InitGameLoopOne();
    ImageRGBA render_target = loop->GetRenderer()->GetImageAsRGBA();
    InitImUi(render_target.width, render_target.height, g_mouse, &loop->frameno);


    if (g_texture_map.slots.len == 0) {
        g_texture_map = InitMap(ctx->a_life, MAX_TEXTURE_B_CNT);
    }
    InitSpriteRenderer(render_target);
    InitFonts(ctx);

    return loop;
}


#endif
