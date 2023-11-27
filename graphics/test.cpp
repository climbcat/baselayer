
/*
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
    GLFWwindow *window = InitGLFW(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);
    
    MouseTrap mouse;
    while (Running(window, &mouse)) {
        XSleep(50);

        RenderRandomPatternRGBA(image, w, h);

        screen.Draw(image, w, h);
        glfwSwapBuffers(window);
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
    GLFWwindow *window = InitGLFW(w, h);
    ScreenQuadTextureProgram screen;
    screen.Init(image, w, h);
    
    MouseTrap mouse;
    Color color_rgba { RGBA_WHITE };
    while (Running(window, &mouse)) {
        XSleep(50);

        RenderLineRGBA(image, w, h, 15, 200, 500, 500, color_rgba);
        RenderLineRGBA(image, w, h, 1000, 10, 15, 500, color_rgba);
        RenderLineRGBA(image, w, h, 400, 200, 900, 200, color_rgba);
        RenderLineRGBA(image, w, h, 800, 10, 800, 500, color_rgba);
        RenderLineRGBA(image, w, h, 920, 120, 900, 790, color_rgba);
        RenderLineRGBA(image, w, h, 920, 120, 950, 790, color_rgba);

        screen.Draw(image, w, h);
        glfwSwapBuffers(window);
    }
}
*/

void TestRandomPCsRotatingBoxes() {
    printf("TestRandomPCsRotatingBoxes\n");

    MArena _b = ArenaCreate();
    MArena *b = &_b;
    u32 width = 1280;
    u32 height = 800;
    GameLoopOne *loop = InitGameLoopOne(b, width, height);

    // data memory pool
    MArena _pointcloud_arena = ArenaCreate();
    MArena *a = &_pointcloud_arena;


    //
    // just set up entities, the rest is automated

    SwRenderer *r = loop->GetRenderer();
    EntitySystem _entity_system = InitEntitySystem();
    EntitySystem *es = &_entity_system;

    Entity *axes = InitAndActivateCoordAxes(es, loop->GetRenderer());
    Entity *box = InitAndActivateAABox(es, { 0.3, 0, 0.7 }, 0.2, r);
    Entity *box2 = InitAndActivateAABox(es, { 0.3, 0.0, -0.7 }, 0.2, r);
    Entity *box3 = InitAndActivateAABox(es, { -0.7, 0, 0.0 }, 0.2, r);

    box->tpe = ET_LINES_ROT;
    box2->tpe = ET_LINES_ROT;
    box3->tpe = ET_LINES_ROT;

    Entity *pc_1;
    {
        RandInit();
        u32 npoints = 90;

        pc_1 = InitAndActivateDataEntity(es, r, a, DT_VERTICES, npoints, 0, NULL);
        pc_1->color = { RGBA_BLUE };

        List<Vector3f> points { (Vector3f*) pc_1->entity_stream->GetData(), npoints };
        Vector3f min { -2, -2, -2 };
        Vector3f max { 2, 2, 2 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f v {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
            points.lst[i] = v;
            points.len++;
        }
    }

    Entity *pc_2;
    {
        RandInit();
        u32 npoints = 300;

        pc_2 = InitAndActivateDataEntity(es, r, a, DT_VERTICES, npoints, 1, pc_1->entity_stream);
        pc_2->color = { RGBA_GREEN };

        List<Vector3f> points { (Vector3f*) pc_2->entity_stream->GetData(), npoints };
        Vector3f min { -2, -2, -2 };
        Vector3f max { 2, 2, 0 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f v {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
            points.lst[i] = v;
            points.len++;
        }
    }
    Entity *pc_3;
    {
        RandInit();
        u32 npoints = 600;

        pc_3 = InitAndActivateDataEntity(es, r, a, DT_VERTICES, npoints, 1, pc_2->entity_stream);
        pc_3->color = { RGBA_RED };

        List<Vector3f> points { (Vector3f*) pc_3->entity_stream->GetData(), npoints };
        Vector3f min { -2, -2, -2 };
        Vector3f max { 0, 0, 0 };
        Vector3f range { max.x - min.x, max.y - min.y, max.z - min.z };
        for (u32 i = 0; i < npoints; ++i) {
            Vector3f v {
                range.x * Rand01_f32() + min.x,
                range.y * Rand01_f32() + min.y,
                range.z * Rand01_f32() + min.z
            };
            points.lst[i] = v;
            points.len++;
        }
    }
    EntitySystemPrint(es);
    EntityStreamPrint(pc_1->entity_stream, (char*) "random point clouds");


    //
    //


    // frame loop
    while (loop->GameLoopRunning()) {
        // E.g.: update entity transforms
        // E.g.: update debug UI
        // E.g.: run simulations
        // E.g.: pull worker thread statuses

        loop->CycleFrame(es);
    }
}

void TestVGROcTree() {
    // we don't really need these numbers, since we shouldn't allocate this much stuff at all
    u8 dm = 5; // max octree depth
    u32 max_cubes = SubCubesTotal(dm);
    u32 max_branches = SubCubesTotal(dm - 1);
    u32 max_leaves = max_cubes - max_branches;

    // AA cube
    Vector3f rootcube_center { 0, 0, 0}; // AABox / octree root-cube center
    float rootcube_radius = 0.2;
    Vector3f c = rootcube_center;
    float r = rootcube_radius;

    // two tmp destinations to avoid allocating max_branches and max_leaves
    MArena _a = ArenaCreate();
    MArena *a = &_a;
    MArena _b = ArenaCreate();
    MArena *b = &_b;

    // imaginary vertex data
    u32 nvertices = 1900;
    List<Vector3f> vertices = InitList<Vector3f>(a, nvertices);
    RandInit();
    for (u32 i = 0; i < nvertices; ++i) {
        Vector3f v {
            rootcube_center.x - r + 2*r*Rand01_f32(),
            rootcube_center.y - r + 2*r*Rand01_f32(),
            rootcube_center.y - r + 2*r*Rand01_f32(),
        };
        vertices.Add(&v);
    }

    // octree data
    // NOTE: these lists much be zerod before use !
    // TODO: use another scheme, this one isn't going to cut it (this commits max_branches of memory)
    List<VGRBranch> branches = InitListOpen<VGRBranch>(a, max_branches);
    List<VGRLeaf> leaves = InitListOpen<VGRLeaf>(b, max_leaves);
    u8 depth_max = dm;
    assert(depth_max >= 2 && "octree min depth is currently 2");

    // assign level 1 branches:
    VGRBranch root;
    VGRBranch *branch = &root;
    root.subcube_block_index = SubCubeAllocBranches(*branch, &branches);
    assert(root.subcube_block_index == 0 && "the first 8 blocks make up level 1 depth branches");
    branches.len = 8;

    // assign 8 level d_max leaves:
    VGRLeaf *leaf;


    // loop points
    Vector3f p;
    for (u32 i = 0; i < vertices.len; ++i) {
        printf("a point %u\n", i);
        p = vertices.lst[i];

        // find descend path
        for (u32 d = 1; d < depth_max; ++d) {
            u8 subcube_idx = SubcubeSelect(p, c, r, &c); // select path
            u32 next_idx = 8*branch->subcube_block_index + subcube_idx;
            branch = branches.lst + next_idx; // step down

            if (branch->subcube_block_index == 0) {
                // encountered unassigned level d+1 branch
                
                if (d < depth_max - 1) {
                    // refer to more branches
                    branch->subcube_block_index = SubCubeAllocBranches(*branch, &branches);
                }
                else {
                    // refer to leaves
                    branch->subcube_block_index = SubCubeAllocLeaves(*branch, &leaves);
                }
            }
        }

        // d == depth_max: select leaf
        u8 subcube_idx = SubcubeSelect(p, c, r, &c); // select path
        u32 leaf_idx = 8*branch->subcube_block_index + subcube_idx;
        leaf = leaves.lst + leaf_idx;
        
        // finally at our leaf, time to put data
        leaf->sum = leaf->sum + p;
        ++leaf->cnt;
    }

    // TODO: write a thing that iterates the octree and pushes its cubes to the renderer, or similar
}


void Test() {
    //TestRandomImageOGL();
    //TestRDrawLines();
    //TestRandomPCsRotatingBoxes();
    TestVGROcTree();
}
