#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__


#include <math.h>
#include <assert.h>

#include "gtypes.h"
#include "entity.h"
#include "geometry.h"

#include "ui.h"
#include "sprite.h"
#include "resource.h"
#include "font.h"
#include "imui.h"

#include "shaders.h"
#include "swrender.h"
#include "gameloop.h"


GameLoopOne *InitGraphics(MContext *ctx) {
    GameLoopOne *loop = InitGameLoopOne();
    ImageRGBA render_target = loop->GetRenderer()->GetImageAsRGBA();
    InitSprites(ctx, render_target);
    InitFonts(ctx);
    InitImUi(render_target.width, render_target.height, g_mouse, &loop->frameno);

    return loop;
}


#endif
