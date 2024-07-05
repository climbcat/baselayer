#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__


#include <math.h>
#include <assert.h>

#include "gtypes.h"
#include "geometry.h"
#include "sprite.h"
#include "atlas.h"
#include "imui.h"

#include "indices.h"
#include "octree.h"

#include "shaders.h"
#include "ui.h"
#include "entity.h"
#include "swrender.h"
#include "gameloop.h"


GameLoopOne *InitGraphics(MContext *ctx) {
    GameLoopOne *loop = InitGameLoopOne();
    ImageRGBA render_target = loop->GetRenderer()->GetImageAsRGBA();
    InitSprites(ctx, render_target);
    InitFonts(ctx);
    InitImUi();

    return loop;
}


#endif
