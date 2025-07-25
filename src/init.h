#ifndef __INIT_H__
#define __INIT_H__


//
// Baselayer initialization


struct MContext {
    // temporary, persistent, lifetime
    MArena _a_tmp;
    MArena _a_pers;
    MArena _a_life;
    MArena *a_tmp;
    MArena *a_pers;
    MArena *a_life;
};

static MContext _g_mctx;
static MContext *g_mctx;
MContext *GetContext(u64 arenas_fixed_size = 0) {
    if (g_mctx == NULL) {
        g_mctx = &_g_mctx;
        g_mctx->_a_tmp = ArenaCreate(arenas_fixed_size);
        g_mctx->a_tmp = &g_mctx->_a_tmp;
        g_mctx->_a_pers = ArenaCreate(arenas_fixed_size);
        g_mctx->a_pers = &g_mctx->_a_pers;
        g_mctx->_a_life = ArenaCreate(arenas_fixed_size);
        g_mctx->a_life = &g_mctx->_a_life;
    }
    return g_mctx;
}


MContext *InitBaselayer() {
    MContext *ctx = GetContext();
    StrSetArenas(ctx->a_tmp, ctx->a_life);
    RandInit();

    return ctx;
}

void BaselayerAssertVersion(u32 major, u32 minor, u32 patch) {
    if (
        BASELAYER_VERSION_MAJOR != major ||
        BASELAYER_VERSION_MINOR != minor ||
        BASELAYER_VERSION_PATCH != patch
    ) {
        assert(1 == 0 && "baselayer version check failed");
    }
}

void BaselayerPrintVersion() {
    printf("%d.%d.%d\n", BASELAYER_VERSION_MAJOR, BASELAYER_VERSION_MINOR, BASELAYER_VERSION_PATCH);
}


#endif
