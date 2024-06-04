#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__


#include <math.h>
#include <assert.h>

#include "gtypes.h"
#include "geometry.h"
#include "atlas.h"

#include "indices.h"
#include "octree.h"

#include "shaders.h"
#include "ui.h"
#include "stream.h"
#include "entity.h"
#include "swrender.h"
#include "gameloop.h"


GameLoopOne *InitGraphics() {
    GameLoopOne *loop = InitGameLoopOne();
    ImageRGBA img = loop->GetRenderer()->GetImageAsRGBA();
    SR_Init(img);
    InitFonts();

    return loop;
}


#endif
