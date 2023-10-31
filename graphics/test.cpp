

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

void TestAnimateBoxesAndOrbitCam() {
    printf("TestAnimateBoxAndOrbitCam\n");

    /*
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
    List<Vector3f> vertex_buffer = InitList<Vector3f>(a, 1000);
    List<Vector2_u16> index_buffer = InitList<Vector2_u16>(a, 1000);
    List<Vector3f> ndc_buffer = InitList<Vector3f>(a, 1000);
    List<Vector2_u16> screen_buffer = InitList<Vector2_u16>(a, 1000);

    // entities
    CoordAxes axes = InitCoordAxes();
    axes._entity.verts_low = vertex_buffer.len;
    axes._entity.lines_low = index_buffer.len;
    CoordAxesGetVerticesAndIndices(axes, &vertex_buffer, &index_buffer);
    axes._entity.verts_high = vertex_buffer.len - 1;
    axes._entity.lines_high = index_buffer.len - 1;

    AABox box = InitAABox({ 0.3, 0, 0.7 }, 0.2);
    box._entity.verts_low = vertex_buffer.len;
    box._entity.lines_low = index_buffer.len;
    AABoxGetVerticesAndIndices(box, &vertex_buffer, &index_buffer);
    box._entity.verts_high = vertex_buffer.len - 1;
    box._entity.lines_high = index_buffer.len - 1;

    AABox box2 = InitAABox({ 0.3, 0.0, -0.7 }, 0.2);
    box2._entity.verts_low = vertex_buffer.len;
    box2._entity.lines_low = index_buffer.len;
    AABoxGetVerticesAndIndices(box2, &vertex_buffer, &index_buffer);
    box2._entity.verts_high = vertex_buffer.len - 1;
    box2._entity.lines_high = index_buffer.len - 1;

    AABox box3 = InitAABox({ -0.7, 0, 0.0 }, 0.2);
    box3._entity.verts_low = vertex_buffer.len;
    box3._entity.lines_low = index_buffer.len;
    AABoxGetVerticesAndIndices(box3, &vertex_buffer, &index_buffer);
    box3._entity.verts_high = vertex_buffer.len - 1;
    box3._entity.lines_high = index_buffer.len - 1;

    axes._entity.next = &box._entity;
    box._entity.next = &box2._entity;
    box2._entity.next = &box3._entity;
    Entity *first = &axes._entity;

    // test the entity chain: 
    u32 eidx = 0;
    Entity *next = first;
    while (next != NULL) {
        printf("%u: vertices %u -> %u lines %u -> %u\n", eidx, next->verts_low, next->verts_high, next->lines_low, next->lines_high);
        eidx++;
        next = next->next;
    }


    // camera
    OrbitCamera cam = InitOrbitCamera(aspect);

    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, false, false, false);
    Matrix4f view, model, mvp;

    u64 iter = 0;
    MouseTrap mouse = InitMouseTrap();
    while (Running(&mouse)) {
        // frame start
        XSleep(33);
        ClearToZeroRGBA(image_buffer, w, h);
        screen_buffer.len = 0;

        // orbit camera
        cam.Update(&mouse, true);

        // entity loop (POC): vertices -> NDC
        u32 eidx = 0;
        Entity *next = first;
        while (next != NULL) {
            if (eidx == 0) {
                model = next->transform;
            }
            else {
                model = next->transform * TransformBuildRotateY(0.03f * iter);
            }
            mvp = TransformBuildMVP(model, cam.view, proj);

            // render
            for (u32 i = next->verts_low; i <= next->verts_high; ++i) {
                ndc_buffer.lst[i] = TransformPerspective(mvp, vertex_buffer.lst[i]);
            }

            eidx++;
            next = next->next;
        }
        ndc_buffer.len = vertex_buffer.len;
        iter++;

        // render / frame end
        u16 nlines_torender = LinesToScreen(w, h, &index_buffer, &ndc_buffer, &screen_buffer);
        for (u32 i = 0; i < screen_buffer.len / 2; ++i) {
            Color color { RGBA_WHITE };
            RenderLineRGBA(image_buffer, w, h, screen_buffer.lst[2*i + 0], screen_buffer.lst[2*i + 1], color);
        }
        screen.Draw(image_buffer, w, h);
        SDL_GL_SwapWindow(window);
    }
    */
}


void Test() {
    //TestRandomImageOGL();
    //TestRDrawLines();
    TestAnimateBoxesAndOrbitCam();
}
