

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
    SDL_Window *window = InitOGL(w, h);
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
    SDL_Window *window = InitOGL(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);
    
    MouseTrap mouse;
    while (Running(&mouse)) {
        XSleep(50);

        RenderLineRGBA(image, w, h, 15, 200, 500, 500);
        RenderLineRGBA(image, w, h, 1000, 10, 15, 500);
        RenderLineRGBA(image, w, h, 400, 200, 900, 200);
        RenderLineRGBA(image, w, h, 800, 10, 800, 500);
        RenderLineRGBA(image, w, h, 920, 120, 900, 790);
        RenderLineRGBA(image, w, h, 920, 120, 950, 790);

        screen.Draw(image, w, h);
        SDL_GL_SwapWindow(window);
    }
}

void TestAnimateBoxAndOrbitCam() {
    printf("TestAnimateBoxAndOrbitCam\n");

    u32 w = 1280;
    u32 h = 800;
    float aspect = (float) w / h;
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    SDL_Window *window = InitOGL(w, h);

    // image buffer
    u8 *image_buffer = (u8*) ArenaAlloc(a, nchannels * w * h);
    ScreenQuadTextureProgram screen;
    screen.Init(image_buffer, w, h);

    // pipeline buffers
    Vector3f *vertex_buffer = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * 100);
    u32 nvertices = 8;
    Vector3f *ndc_buffer = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * 100);
    Vector2_u16 *lines_idxbuffer = (Vector2_u16*) ArenaAlloc(a, sizeof(Vector2_u16) * 1000);
    u32 nlines = 12;
    Vector2_u16 *lines_screen_buffer = (Vector2_u16*) ArenaAlloc(a, sizeof(Vector2_u16) * 1000);

    // entities
    Vector3f box_center = Vector3f {0, 0, 0}; // local coords
    AABox box { box_center, 0.3 };
    Vector3f box_position { 0, 0, 0 };
    Matrix4f box_transform = TransformBuild(y_hat, 0, box_position);
    OrbitCamera cam = InitOrbitCamera(aspect);
    Vector3f cam_position { -8, 1, 1 };

    // populate vertex & line buffer
    AABoxGetCorners(box, vertex_buffer);
    AABoxGetLinesIndices(0, lines_idxbuffer);

    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f view = TransformBuild(y_hat, 0, cam_position) * TransformBuildLookRotationYUp(box_position, cam_position); 
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f model = box_transform;
    Matrix4f mvp;

    // orbit camera params
    u64 iter = 0;
    MouseTrap mouse = InitMouseTrap();
    float mouse2theta = 0.2;
    float mouse2phi = 0.2;
    while (Running(&mouse)) {
        XSleep(10);
        ClearToZeroRGBA(image_buffer, w, h);

        // contact the mouse
        printf("left: %d right: %d x: %d y: %d dx: %d dy: %d up: %d down: %d\n", mouse.held, mouse.rheld, mouse.x, mouse.y, mouse.dx, mouse.dy, mouse.wup, mouse.wdown);

        // box rotation
        model = box_transform * TransformBuildRotateY(0.03f * iter);

        // orbitcam
        cam.Update(&mouse);
        mvp = TransformBuildMVP(model, cam.view, proj);
        iter++;

        // render
        for (u32 i = 0; i < nvertices; ++i) {
            ndc_buffer[i] = TransformPerspective(mvp, vertex_buffer[i]);
        }
        u16 nlines_torender = LinesToScreen(w, h, lines_idxbuffer, nlines, ndc_buffer, lines_screen_buffer);
        for (u32 i = 0; i < nlines_torender; ++i) {
            RenderLineRGBA(image_buffer, w, h, lines_screen_buffer[2*i + 0], lines_screen_buffer[2*i + 1]);
        }
        screen.Draw(image_buffer, w, h);
        SDL_GL_SwapWindow(window);
    }
}


void Test() {
    //TestRandomImageOGL();
    //TestRDrawLines();
    TestAnimateBoxAndOrbitCam();
}
