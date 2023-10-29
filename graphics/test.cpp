
void TestPointCloudPerspectiveProj_2() {
    printf("TestPointCloudPerspectiveProj\n");

    // Working with the "ogl" proj matrix formula.
    //
    // Now, the camera is pointing in the negative z direction! 
    //
    // The clipping works in both the far and near planes:
    // to see this, collapse the pc z range and play with the new/far
    // ranges until til pc just appears/disappears in the output
    // image.
    //
    // Regarding the z-negative direction; This is not not practical:
    // If cameras view "backwards" could we find a proj matrix that 
    // invert this? Should be a "flip transform" that flips this direction
    // and have it be baked into the proj matrix. This amounts to flipping
    // some of the signs in the proj matrix formula.... (PerspectiveMatrixOpenGL)


    MArena arena = ArenaCreate();
    MArena *a = &arena;

    // define the point cloud    
    f32 min_x = 0;
    f32 min_y = 0;
    f32 min_z = 2;
    f32 range_pc = 1;
    f32 range_x = 0.5;
    f32 range_y = 0.5;
    f32 range_z = 0.01;

    u32 npoints = 128;
    Vector3f *pc = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * npoints);

    RandInit();
    Vector3f v;
    for (u32 i = 0; i < npoints; ++i) {
        v.x = Rand01() * range_x + min_x;
        v.y = Rand01() * range_y + min_y;
        v.z = Rand01() * range_z + min_z;
        pc[i] = v;
    }

    // camera projection
    PerspectiveFrustum frustum;
    frustum.fov = 90;
    frustum.aspect = 1;
    frustum.dist_near = 0.1;
    frustum.dist_far = 6;
    Matrix4f proj = PerspectiveMatrixOpenGL(frustum, false, false, true);

    // camera position
    Vector3f campos { 0, 0, 0 };
    f32 angle_y = 0;

    // camera MVP matrix (model -> view -> proj with model = Id right now)
    Matrix4f view = TransformBuild(Vector3f {0, 1, 0}, angle_y, campos);
    Matrix4f world2cam = TransformGetInverse(view);
    Matrix4f MVP = proj * world2cam;

    // camera sensor / pixel space
    u32 pos_x;
    u32 pos_y;
    u32 w = 400;
    u32 h = 400;
    u8 *image = (u8*) ArenaAlloc(a, w * h);

    // render points to bitmap
    for (u32 i = 0; i < npoints; ++i) {
        Vector3f org = pc[i];
        Vector3f v = TransformPerspective(MVP, org);

        if (v.z < -1 || v.z > 1) {
            continue;
        }

        pos_x = (u32) ((v.x + 1) / 2 * w);
        pos_y = (u32) ((v.y + 1) / 2 * h);

        if (!Cull(pos_x, pos_y, w, h)) {
            image[Pos2Idx(pos_x, pos_y, w)] = 255;
        }
    }

    u32 nchannels = 1;
    stbi_write_png("perspective.png", w, h, nchannels, image, 1 * w);
}


void TestRotateCamera() {
    printf("TestRotateCamera\n");
    MArena arena = ArenaCreate();
    MArena *a = &arena;

    // define the point cloud    
    f32 min_x = -0.25;
    f32 min_y = 0.5;
    f32 min_z = -0.25;
    f32 range_x = 0.5;
    f32 range_y = 0.5;
    f32 range_z = 0.5;

    u32 npoints = 128;
    Vector3f *pc = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * npoints);

    RandInit();
    Vector3f v;
    for (u32 i = 0; i < npoints; ++i) {
        v.x = Rand01() * range_x + min_x;
        v.y = Rand01() * range_y + min_y;
        v.z = Rand01() * range_z + min_z;
        pc[i] = v;
    }

    //// define the "camera" volume / box thingee --> this is not NDC === [-1,1]^2
    f32 cam_min_x = -1;
    f32 cam_min_y = -1;
    f32 cam_min_z = -1;
    f32 cam_range_x = 2;
    f32 cam_range_y = 2;
    f32 cam_range_z = 2;

    // camera projection
    PerspectiveFrustum frustum;
    frustum.fov = 90;
    frustum.aspect = 1;
    frustum.dist_near = 0.001;
    frustum.dist_far = 100;
    Matrix4f proj = PerspectiveMatrix(frustum);

        // camera sensor / pixel space
    u32 w = 400;
    u32 h = 400;

    for (int i = 0; i < 90; ++i) {
        // camera position
        Vector3f campos { 0, 0, 2 };
        f32 angle_y = i;

        // camera MVP matrix (model -> view -> proj with model = Id right now)
        Matrix4f view = TransformBuild(Vector3f {0, 1, 0}, angle_y * deg2rad, campos);
        Matrix4f MVP = proj * view;

        u8 *image = (u8*) ArenaAlloc(a, w * h);

        // render points to bitmap
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f org = pc[i];
            Vector3f v = TransformPerspective(MVP, org);

            // cull z value: 
            if (v.z < cam_min_z || v.z > cam_min_z + cam_range_z) {
                continue;
            }

            u32 pos_x = (u32) ((v.x - cam_min_x) / cam_range_x * w);
            u32 pos_y = (u32) ((v.y - cam_min_y) / cam_range_y * h);

            if (!Cull(pos_x, pos_y, w, h)) {
                image[Pos2Idx(pos_x, pos_y, w)] = 255;
            }
        }

        u32 nchannels = 1;
        char buff[200];
        sprintf(buff, "perspective_%.2f.png", angle_y);
        printf("writing: %s\n", buff);
        stbi_write_png(buff, w, h, nchannels, image, 1 * w);
    }
}

void TestPointCloudBoxProj() {
    printf("TestPointCloud\n");

    MArena arena = ArenaCreate();
    MArena *a = &arena;

    // define the point cloud    
    f32 min_x = 0.2;
    f32 min_y = 0.2;
    f32 min_z = 0.2;

    f32 range_x = 1;
    f32 range_y = 1;
    f32 range_z = 1;

    u32 npoints = 128;
    Vector3f *pc = (Vector3f*) ArenaAlloc(a, sizeof(Vector3f) * npoints);
    
    RandInit();
    Vector3f v;
    for (u32 i = 0; i < npoints; ++i) {
        v.x = Rand01() * range_x + min_x;
        v.y = Rand01() * range_y + min_y;
        v.z = Rand01() * range_z + min_z;
        pc[i] = v;
    }

    // define the "camera" volume / box thingee
    u32 cam_min_x = 0;
    u32 cam_min_y = 0;
    u32 cam_min_z = 0;

    u32 cam_range_x = 1.4;
    u32 cam_range_y = 1.1;
    u32 cam_range_z = 1.7;

    // define the sensor space
    // NOTE: If OpenGL, we need to go further into normalized screen space [-1,1]x[-1,1]
    u32 w = 400;
    u32 h = 400;
    u8 *image = (u8*) ArenaAlloc(a, w * h);

    // render points
    for (u32 i = 0; i < npoints; ++i) {
        v = pc[i];

        u32 pos_x = (u32) ((v.x - cam_min_x) / cam_range_x * w);
        u32 pos_y = (u32) ((v.y - cam_min_y) / cam_range_y * h);

        if (!Cull(pos_x, pos_y, w, h)) {
            image[Pos2Idx(pos_x, pos_y, w)] = 255;
        }
    }

    u32 nchannels = 1;
    stbi_write_png("pc_xy.png", w, h, nchannels, image, 1 * w);
}

void TestRandImage() {
    printf("TestRandImage\n");

    MArena arena = ArenaCreate();
    MArena *a = &arena;
    RandInit();

    u32 w = 640;
    u32 h = 480;
    u8 *image = (u8*) ArenaAlloc(a, w * h);
    u32 idx;
    for (u32 i = 0; i < w; ++i) {
        for (u32 j = 0; j < h; ++j) {
            idx = i + j * w;
            if (idx % 2 == 0) {
                image[idx] = RandIntMax(255);
            }
        }
    }

    u32 nchannels = 1;
    stbi_write_png("written.png", w, h, nchannels, image, 1 * w);
}


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


void TestAnimateClockHand() {
    printf("TestRDrawAnimatePrecession\n");

    u32 w = 1280;
    u32 h = 800;
    u32 nchannels = 4;
    MArena arena = ArenaCreate();
    MArena *a = &arena;
    u8 *image = (u8*) ArenaAlloc(a, nchannels * w * h);
    SDL_Window *window = InitOGL(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);
    
    float angle = 0;
    float delta_angle = 0.1;
    u16 cx = w/2;
    u16 cy = h/2;
    u16 px = 0;
    u16 py = 0;
    u16 armlen = 400;
    MouseTrap mouse;
    while (Running(&mouse)) {
        XSleep(33);
        ClearToZeroRGBA(image, w, h);

        angle += delta_angle;
        px = floor( cx + cos(angle) * armlen );
        py = floor( cy + sin(angle) * armlen );
        RenderLineRGBA(image, w, h, cx, cy, px, py);

        screen.Draw(image, w, h);
        SDL_GL_SwapWindow(window);
    }
}

void TestAnimateBoxAndLookrot() {
    printf("TestAnimateBox\n");

    // tests validity of camera translation and of object transform, including local object rotation

    u32 w = 1280;
    u32 h = 800;
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
    Vector3f box_position { 0, -2, 5 };
    Matrix4f box_transform = TransformBuild(y_hat, 0, box_position);
    Camera cam { PerspectiveFrustum { 90, (float) w / h, 0.1, 20 } }; // fov, aspect, near, far
    Vector3f cam_position { 2.2, 1.2, -5 };

    // populate vertex & line buffer
    AABoxGetCorners(box, vertex_buffer);
    AABoxGetLinesIndices(0, lines_idxbuffer);


    // NOTES: 
    //  Transforms are local2world, also the view transform. However the BuildMVP() function
    //  inverts the view matrix, before multiplying into an mvp. 
    //  We are multiplying from the right, so the right-most matrix / transform applied to vertices
    //  is applied to the local coordinates.
    //  This is how the box rotation animation works currently; by applying a rotation-only matrix before
    //  the formal "box transform" is applied.


    // build transform: [ model -> world -> view_inv -> projection ]
    Matrix4f view = TransformBuild(y_hat, 0, cam_position); // TODO: LOOKAT to incorporate camera view direction
    Matrix4f view_lookat = view * TransformBuildLookRotationYUp(box_position, cam_position); 
    Matrix4f proj = PerspectiveMatrixOpenGL(cam.frustum, true, false, true);
    Matrix4f model = box_transform;
    Matrix4f mvp = TransformBuildMVP(model, view, proj);

    u64 iter = 0;
    MouseTrap mouse;
    while (Running(&mouse)) {
        XSleep(10);
        ClearToZeroRGBA(image_buffer, w, h);

        model = box_transform * TransformBuildRotateY(0.03f * iter);
        if (iter > 50) {
            view = view_lookat;
        }
        mvp = TransformBuildMVP(model, view, proj);
        iter++;

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
    //TestRandImage();
    //TestPointCloudBoxProj();
    //TestRotateCamera();
    //TestPointCloudPerspectiveProj_2();
    //TestRandomImageOGL();
    //TestRDrawLines();
    //TestAnimateClockHand();
    TestAnimateBoxAndLookrot();
}
