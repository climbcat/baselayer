

void TestRandomImageOGL() {
    printf("TestRandomImageOGL\n");

    // setup image data
    u32 w = 1280;
    u32 h = 800;
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    u8 *image = (u8*) ArenaAlloc(a, nchannels * w * h);

    // OGL window & sharder
    SDL_Window *window = InitSDL(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);
    
    MouseTrap mouse;
    while (Running(&mouse)) {
        XSleep(50);

        RenderRandomPatternRGBA(image, w, h);

        screen.Draw(image, w, h);
        SDL_GL_SwapWindow(window);
    }

}


void TestRDrawLines() {
    printf("TestRDrawLines\n");

    u32 w = 1280;
    u32 h = 800;
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    u8 *image = (u8*) ArenaAlloc(a, nchannels * w * h);
    SDL_Window *window = InitSDL(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);
    
    MouseTrap mouse;
    Color color_rgba { RGBA_WHITE };
    while (Running(&mouse)) {
        XSleep(50);

        RenderLineRGBA(image, w, h, 15, 200, 500, 500, color_rgba);
        RenderLineRGBA(image, w, h, 1000, 10, 15, 500, color_rgba);
        RenderLineRGBA(image, w, h, 400, 200, 900, 200, color_rgba);
        RenderLineRGBA(image, w, h, 800, 10, 800, 500, color_rgba);
        RenderLineRGBA(image, w, h, 920, 120, 900, 790, color_rgba);
        RenderLineRGBA(image, w, h, 920, 120, 950, 790, color_rgba);

        screen.Draw(image, w, h);
        SDL_GL_SwapWindow(window);
    }
}

void TestLoadMatrics() {
    u64 size_bytes01;
    u64 size_bytes02;
    u8* data01 = LoadFileMMAP("../data/231101_positions01_m4x4.bin", &size_bytes01);
    u8* data02 = LoadFileMMAP("../data/231101_positions02_m4x4.bin", &size_bytes02);

    assert((size_bytes01 % sizeof(Matrix4f)) == 0 && "check whole number of matrices");
    assert((size_bytes02 % sizeof(Matrix4f)) == 0 && "check whole number of matrices");

    u32 cnt01 = size_bytes01 / sizeof(Matrix4f);
    u32 cnt02 = size_bytes02 / sizeof(Matrix4f);

    List<Matrix4f> matrices01 { (Matrix4f*) data01, cnt01 };
    List<Matrix4f> matrices02 { (Matrix4f*) data01, cnt02 };

    printf("\nLoaded %d transforms in 231101_positions01_m4x4.bin: \n", cnt01);
    for (u32 i = 0; i < matrices01.len; ++i) {
        PrintMatrix4d(matrices01.lst + i);
    }

    printf("\nLoaded %d transforms in 231101_positions01_m4x4.bin: \n", cnt02);
    for (u32 i = 0; i < matrices02.len; ++i) {
        PrintMatrix4d(matrices02.lst + i);
    }
}

void Test() {
    //TestRandomImageOGL();
    //TestRDrawLines();
    TestLoadMatrics();
}
