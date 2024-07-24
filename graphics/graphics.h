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


GameLoopOne *InitGraphics(MContext *ctx) {

    //
    // init subsustem; memory, context, ptrs

    GameLoopOne *loop = InitGameLoopOne();
    ImageRGBA render_target = loop->GetRenderer()->GetImageAsRGBA();
    InitImUi(render_target.width, render_target.height, g_mouse, &loop->frameno);
    InitSpriteRenderer(render_target);

    //
    // load typed resources such as FontAtlas, SpriteMap

    g_texture_map = InitMap(ctx->a_life, MAX_RESOURCE_CNT);
    g_resource_map = InitMap(ctx->a_life, MAX_RESOURCE_CNT);

    // load & check resource file
    ResourceStreamHandle hdl = ResourceStreamLoadAndOpen(ctx->a_tmp, ctx->a_life, "all.res");
    g_font_names = hdl.names[RT_FONT];

    // map out the resources
    ResourceHdr *res = hdl.first;
    while (res) {
        // fonts
        if (res->tpe == RT_FONT) {
            FontAtlas *font = FontAtlasLoadBinaryStream(res->GetInlinedData(), res->data_sz);
            font->Print();

            MapPut(&g_resource_map, font->GetKey(), font);
            MapPut(&g_texture_map, font->GetKey(), &font->texture);
        }

        // sprite maps
        else if (res->tpe == RT_SPRITE) {
            SpriteMap *smap = SpriteMapLoadStream((u8*) res->GetInlinedData(), res->data_sz);
            printf("sprite map: %s, %s, count: %u, atlas w: %u, atlas h: %u\n", smap->map_name, smap->key_name, smap->sprites.len, smap->texture.width, smap->texture.height);

            MapPut(&g_resource_map, smap->key_name, smap);
            MapPut(&g_texture_map, smap->key_name, &smap->texture);
        }

        // other
        else {
            printf("WARN: unknown resource detected\n");
        }

        // iter
        res = res->GetInlinedNext();
    }
    SetFontAndSize(FS_48, g_font_names->GetStr());


    return loop;
}


#endif
