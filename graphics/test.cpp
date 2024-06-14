#include "../baselayer.h"
#include "graphics.h"


List<Vector3f> CreateRandomPointCloud(MArena *a, u32 npoints, Vector3f center, Vector3f dimensions) {
    List<Vector3f> pc = InitList<Vector3f>(a, npoints);
    RandInit();
    Vector3f dims = dimensions;
    Vector3f min { center.x - dims.x * 0.5f, center.y - dims.y * 0.5f, center.z - dims.z * 0.5f };
    for (u32 i = 0; i < npoints; ++i) {
        Vector3f v {
            dims.x * Rand01_f32() + min.x,
            dims.y * Rand01_f32() + min.y,
            dims.z * Rand01_f32() + min.z
        };
        pc.Add(&v);
    }
    return pc;
}


void TestRandomPCWithNormals() {
    printf("TestRandomPCWithNormals\n");

    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();
    Entity *axes = EntityCoordAxes(es, NULL, r);
    
    MArena _a_pointclouds = ArenaCreate();
    MArena *a_pcs = &_a_pointclouds;

    // point clouds 
    List<Vector3f> points = CreateRandomPointCloud(a_pcs, 90, { 0.0f, 0.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
    List<Vector3f> normals = CreateRandomPointCloud(a_pcs, 90, { 0.1f, 0.1f, 0.1f }, { 0.05f, 0.05f, 0.05f });

    Entity *pc = EntityPoints(es, NULL, points);
    pc->ext_normals = &normals;
    pc->tpe = ET_POINTCLOUD_W_NORMALS;
    pc->color = Color { RGBA_GREEN };
    pc->color_alt = Color { RGBA_BLUE };

    EntitySystemPrint(es);

    loop->JustRun(es);
}


void TestVGROcTree() {
    printf("TestVGROcTree\n");


    // test parameters
    float rootcube_radius = 0.2;
    //float leaf_size = rootcube_radius / 2.0 / 2.0 / 2.0 / 2.0;
    //float leaf_size = rootcube_radius / 2.0 / 2.0;

    // TODO: there is a bug, there should be about 8 vertices out for this case, we get 56. !
    float leaf_size = rootcube_radius;
    u32 nvertices_src = 100;
    u32 nvertices_src_2 = 1000;
    bool display_boxes = false;
    Color color_in { RGBA_BLUE };
    Color color_in_2 { RGBA_BLUE };
    Color color_out { RGBA_RED };
    Matrix4f box_transform = Matrix4f_Identity();
    Matrix4f src_transform = Matrix4f_Identity();
    Matrix4f src_transform_2 = Matrix4f_Identity();


    // src/dst storage
    MArena _a_tmp = ArenaCreate(); // vertices & branch list location
    MArena *a_tmp = &_a_tmp;
    MArena _a_dest = ArenaCreate(); // dest storage
    MArena *a_dest = &_a_dest;

    List<Vector3f> src = InitList<Vector3f>(a_tmp, nvertices_src);
    Vector3f rootcube_center { 0, 0, 0 };
    float pc_radius = 0.2;
    RandInit(913424423);
    for (u32 i = 0; i < nvertices_src; ++i) {
        Vector3f v {
            rootcube_center.x - pc_radius + 2*pc_radius*Rand01_f32(),
            rootcube_center.y - pc_radius + 2*pc_radius*Rand01_f32(),
            rootcube_center.y - pc_radius + 2*pc_radius*Rand01_f32(),
        };
        src.Add(&v);
    }
    List<Vector3f> src_2 = InitList<Vector3f>(a_tmp, nvertices_src_2);
    Vector3f rootcube_center_2 { 0.1, 0.1, 0.1 };
    float pc_radius_2 = 0.1;
    RandInit(913424423);
    for (u32 i = 0; i < nvertices_src_2; ++i) {
        Vector3f v {
            rootcube_center.x - pc_radius_2 + 2*pc_radius_2*Rand01_f32(),
            rootcube_center.y - pc_radius_2 + 2*pc_radius_2*Rand01_f32(),
            rootcube_center.y - pc_radius_2 + 2*pc_radius_2*Rand01_f32(),
        };
        src_2.Add(&v);
    }


    // run the vgr
    List<Vector3f> dest = InitList<Vector3f>(a_dest, nvertices_src);
    OcTreeStats stats;
    List<OcLeaf> leaf_blocks_out;
    List<OcBranch> branch_blocks_out;

    // run the *other* vgr
    VoxelGridReduce vgr = VoxelGridReduceInit(leaf_size, rootcube_radius, box_transform);
    vgr.AddPoints(src, src, src_transform);
    vgr.AddPoints(src_2, src_2, src_transform_2);
    List<Vector3f> points_out;
    List<Vector3f> normals_out;
    vgr.GetPoints(a_dest, &points_out, &normals_out);
    leaf_blocks_out = vgr.leaves;
    branch_blocks_out = vgr.branches;


    // visualize
    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();
    Entity *axes = EntityCoordAxes(es, NULL, r);

    Entity *src_pc = EntityPoints(es, NULL, src);
    src_pc->transform = src_transform;
    src_pc->tpe = ET_POINTCLOUD;
    src_pc->color = color_in;

    Entity *src_pc_2 = EntityPoints(es, NULL, src_2);
    src_pc_2->transform = src_transform_2;
    src_pc_2->tpe = ET_POINTCLOUD;
    src_pc_2->color = color_in_2;

    Entity *dest_pc = EntityPoints(es, NULL, points_out);
    src_pc->transform = src_transform;
    dest_pc->tpe = ET_POINTCLOUD;
    dest_pc->color = color_out;

    // print leaf boxes
    printf("\n");
    for (u32 i = 0; i < leaf_blocks_out.len; ++i) {
        OcLeaf leaf = leaf_blocks_out.lst[i];

        for (u32 j = 0; j < 8; ++j) {
            printf("%u ", leaf.cnt[j]);
        }
        #if VGR_DEBUG
        for (u32 j = 0; j < 8; ++j) {
            if (leaf.cnt[j] > 0) {
                printf("c: %f %f %f, r: %f\n", leaf.center[j].x, leaf.center[j].y, leaf.center[j].z, leaf.radius[j]);
                if (display_boxes) {
                    Entity *cub = EntityAABox(es, leaf.center[j], leaf.radius[j], r);
                }
            }
        }
        #endif
    }

    // print vgr stats
    printf("\n");
    vgr.stats.Print();

    loop->JustRun(es);
}


void TestQuaternionRotMult() {
    printf("TestQuaternionRotMult\n");

    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();

    Entity *axes = EntityCoordAxes(es, NULL, r);
    Entity *box0 = EntityAABox(es, NULL, { 0.0f, 0.0f, 0.f }, 0.2f, r);
    box0->color = Color { RGBA_RED };
    Entity *box = EntityAABox(es, NULL, { 0.0f, 0.0f, 0.f }, 0.2f, r);

    Vector3f r1 { 0.0f, 1.0f, 0.0f };
    r1.Normalize();
    float angle0 = 30.0f * deg2rad;
    float angle1 = 15.0f * deg2rad;
    Quat q1 = QuatAxisAngle(r1, angle1);
    box0->transform = TransformBuild(r1, angle0);
    box->transform = TransformQuaternion( QuatMult(q1, q1) );

    loop->JustRun(es);
}


void TestSlerpAndMat2Quat() {
    printf("TestSlerpAndMat2Quat\n");

    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();

    Entity *axes = EntityCoordAxes(es, NULL, r);

    Entity *box0 = EntityAABox(es, NULL, { 0.0f, 0.0f, 0.f }, 0.2f, r);
    box0->color = Color { RGBA_RED };
    Entity *box1 = EntityAABox(es, NULL, { 0.0f, 0.0f, 0.f }, 0.2f, r);
    box1->color = Color { RGBA_BLUE };
    Entity *box = EntityAABox(es, NULL, { 0.0f, 0.0f, 0.f }, 0.2f, r);
    box->color = Color { RGBA_GREEN };

    Vector3f r1 { 0.1f, 1.0f, -0.4f };
    r1.Normalize();
    float angle0 = 5.0f * deg2rad;
    float angle1 = 65.0f * deg2rad;


    box0->transform = TransformBuild(r1, angle0);
    box1->transform = TransformBuild(r1, angle1);
    box->transform = TransformBuild(r1, angle0);

    Quat q0 = QuatFromTransform(box0->transform);
    Quat q1 = QuatFromTransform(box1->transform);
    Quat q = q0;

    float t = 0;
    float dt = 0.03;
    while (loop->GameLoopRunning()) {

        t += dt;
        if ( t <= 0.001f || t >= 0.999f ) {
            dt *= -1;
            t += dt;
        }
        q = Slerp(q0, q1, t);
        box->transform = TransformQuaternion( q );

        loop->CycleFrame(es);
    }
    loop->Terminate();
}


void TestPointCloudsBoxesAndSceneGraph() {
    printf("TestPointCloudsBoxesAndSceneGraph\n");

    // point clouds 
    MArena _a_pointclouds = ArenaCreate();
    MArena *a_pcs = &_a_pointclouds;
    List<Vector3f> points_1 = CreateRandomPointCloud(a_pcs, 90, { 0.0f, 0.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
    List<Vector3f> points_2 = CreateRandomPointCloud(a_pcs, 300, { 0.0f, 0.0f, -1.f }, { 4.0f, 4.0f, 2.0f });
    List<Vector3f> points_3 = CreateRandomPointCloud(a_pcs, 300, { -1.0f, -1.0f, -1.f }, { 2.0f, 2.0f, 2.0f });

    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();

    // entities
    Entity *axes = EntityCoordAxes(es, NULL, r);
    Entity *box = EntityAABox(es, NULL, { 0.3f, 0.0f, 0.7f }, 0.2f, r);
    Entity *box2 = EntityAABox(es, NULL, { 0.3f, 0.0f, -0.7f }, 0.2f, r);
    Entity *box3 = EntityAABox(es, NULL, { -0.7f, 0.0f, 0.0f }, 0.2f, r);

    box->tpe = ET_LINES_ROT;
    box2->tpe = ET_LINES_ROT;
    box3->tpe = ET_LINES_ROT;

    Entity *pc_1 = EntityPoints(es, axes, points_1);
    Entity *pc_2 = EntityPoints(es, pc_1, points_2);
    Entity *pc_3 = EntityPoints(es, pc_2, points_3);

    pc_1->color = { RGBA_GREEN };
    pc_2->color = { RGBA_BLUE };
    pc_3->color = { RGBA_RED };


    // test point colors
    RandInit();
    List<Color> point_colors = InitList<Color>(a_pcs, points_1.len);
    for (u32 i = 0; i < points_1.len; ++i) {
        Color c;
        c.a = 255;
        c.r = RandMinMaxI(0, 255);
        c.g = RandMinMaxI(0, 255);
        c.b = RandMinMaxI(0, 255);
        point_colors.Add(c);
    }
    pc_1->SetColorsList(point_colors);


    // test render only active entities
    pc_2->DeActivate();
    EntitySystemPrint(es);
    while (loop->GameLoopRunning()) {
        Key kpress = loop->mouse.last_keypress_frame;

        if (kpress == OUR_GLFW_KEY_LEFT) {
            es->CursorPrev();
        }
        else if (kpress == OUR_GLFW_KEY_RIGHT) {
            es->CursorNext();
        }
        else if (kpress == OUR_GLFW_KEY_UP) {
            es->CursorUp();
        }
        else if (kpress == OUR_GLFW_KEY_DOWN) {
            es->CursorDown();
        }
        else if (kpress == OUR_GLFW_KEY_SPACE) {
            es->CursorToggleEntityActive();
        }

        loop->CycleFrame(es);
    }
    loop->Terminate();
}


void _PrintIndices(const char *prefix, List<u32> idxs) {
    printf("%s", prefix);
    for (u32 i = 0; i < idxs.len; ++i) {
        printf("%u ", idxs.lst[i]);
    }
    printf("\n");
}
void TestIndexSetOperations() {
    printf("TestIndexSetOperations\n");

    MContext *ctx = InitBaselayer();
    MArena *a = ctx->a_tmp;

    // naiive values
    List<u32> values = InitList<u32>(a, 10);
    for (u32 i = 0; i < 10; ++i) {
        values.Add( i );
    }

    // covering indices with duplicates
    List<u32> idxs = InitList<u32>(a, 15);
    for (u32 i = 0; i < 15; ++i) {
        idxs.Add( i % 10 );
    }

    // list to remove
    List<u32> idxs_rm = InitList<u32>(a, 15);
    idxs_rm.Add( (u32) 0 );
    idxs_rm.Add( 4 );
    idxs_rm.Add( 5 );
    idxs_rm.Add( 6 );
    idxs_rm.Add( 7 );

    // list to extract
    List<u32> idxs_extr = InitList<u32>(a, 15);
    idxs_extr.Add( 9 );
    idxs_extr.Add( 5 );
    idxs_extr.Add( 1 );
    idxs_extr.Add( (u32) 0 );
    idxs_extr.Add( 2 );

    printf("\n");
    _PrintIndices("values:    ", values);
    _PrintIndices("idxs:      ", idxs);
    _PrintIndices("idxs_rm:   ", idxs_rm);
    _PrintIndices("idxs_extr: ", idxs_extr);

    // extract
    List<u32> vals_extr_out;
    List<u32> idxs_extr_out;
    List<u32> indirect = IndicesExtract<u32>(a, a, values, idxs_extr, &vals_extr_out, &idxs_extr_out);

    printf("\ntest IndicesExtract()\n");
    _PrintIndices("vals_extr_out: ", vals_extr_out);
    _PrintIndices("idxs_extr_out: ", idxs_extr_out);

    //: remove
    List<u32> vals_rm_out;
    List<u32> idxs_rm_out;
    List<u32> indirect_rm = IndicesRemove<u32>(a, a, values, idxs, idxs_rm, &vals_rm_out, &idxs_rm_out);
    printf("\ntest IndicesRemove()\n");
    _PrintIndices("vals_rm_out: ", vals_rm_out);
    _PrintIndices("idxs_rm_out: ", idxs_rm_out);
}


void TestLayoutGlyphQuads() {
    printf("TestLayoutGlyphQuads\n");

    MContext *ctx = InitBaselayer();
    GameLoopOne *loop = InitGraphics();

    LayoutText("The quick brown fox jumps over the lazy dog", 50, 100, 1000, 200, ColorRandom());
    LayoutText("The other quick brown fox jumps over the other lazy dog", 100, 200, 400, 200, ColorRandom());

    SR_Render();
    loop->JustShowBuffer();
}


void TestBrownianGlyphs() {
    printf("TestBrownianGlyphs\n");

    MContext *ctx = InitBaselayer();
    GameLoopOne *loop = InitGraphics();


    Color clbl = ColorRandom();
    u32 ncols = 100;
    List<Color> cols = InitList<Color>(ctx->a_pers, ncols);
    for (u32 i = 0; i < ncols; ++i) {
        cols.Add( ColorRandom() );
    }

    List<QuadHexaVertex> quads = LayoutText(ctx->a_pers, "The quick brown fox jumps over the lazy dog", 470, 300, 300, 300, FS_72, TAL_CENTER);
    DrawCall dc { 0, quads };
    for (u32 i = 0; i < quads.len; ++i) {
        QuadHexaVertex *q = quads.lst + i;
        for (u32 j = 0; j < 6; ++j) {
            QuadVertex *v = q->verts + j;
            v->col = cols.lst[i % cols.len];
        }
    }
    List<QuadHexaVertex> quads_initial = ListCopy(ctx->a_life, quads);

    f32 magnitude = 0.8;
    while (loop->GameLoopRunning()) {
        loop->FrameStart2D();

        // brownian motion applied to each character, effect
        for (u32 i = 0; i < quads.len; ++i) {
            QuadHexaVertex *q = quads.lst + i;
            Vector2f d { magnitude * RandPM1_f32(), magnitude * RandPM1_f32() };
            for (u32 j = 0; j < 6; ++j) {
                QuadVertex *v = q->verts + j;
                v->pos = v->pos + d;
            }
        }

        // reset on key_space
        Key k = loop->GetMouseTrap()->last_keypress_frame;
        if (k == OUR_GLFW_KEY_SPACE) {
            _memcpy(quads.lst, quads_initial.lst, sizeof(QuadHexaVertex) * quads_initial.len);
        }
        else if (k == OUR_GLFW_KEY_UP) {
            magnitude *= 1.5f;
        }
        else if (k == OUR_GLFW_KEY_DOWN) {
            magnitude /= 1.5f;
        }

        // render
        LayoutText("press space/up/down", 50, 50, 1000, 200, clbl, FS_30);
        SR_Push(dc);

        loop->FrameEnd2D();
    }
    loop->Terminate();
}


void TestUIDragPanel() {
    printf("TestUIPanel\n");

    MContext *ctx = InitBaselayer();
    GameLoopOne *loop = InitGraphics();

    s32 l = 180;
    s32 t = 140;
    s32 w = 400;
    s32 h = 250;
    s32 border = 1;
    f32 graynexx;
    bool show_pnl = true;


    while (loop->GameLoopRunning()) {
        loop->FrameStart2D(ColorGray(0.9f));

        LayoutText("The quick brown fox jumps over the lazy dog", 50, 80, 1000, 200, ColorBlack());

        // bordered drag-panel
        if (show_pnl) {
            graynexx = 0.8f;
            border = 1;
            if (loop->mouse.LimsLTWHLastFrame(l, t, w, h)) {
                graynexx = 0.7f;

                if (loop->mouse.l) {
                    graynexx = 0.6f;
                    border = 2;
                    l += loop->mouse.dx;
                    t += loop->mouse.dy;
                }
                bool clicked = loop->mouse.ClickedRecently();
                if (clicked) {
                    printf("click\n");
                    //show_pnl = false;
                }
            }
            LayoutPanel(l, t, w, h, border, ColorBlack(), ColorGray(graynexx));
            LayoutText("The other quick brown fox jumps over the other lazy dog", l, t, w, h, ColorBlack(), FS_30, TAL_CENTER);
        }

        if (loop->GetMouseTrap()->last_keypress_frame == OUR_GLFW_KEY_SPACE) {
            show_pnl = true;
        }

        loop->FrameEnd2D();
    }
    loop->Terminate();

    // TODO: draw a boundaried panel - OK
    // TODO: make it respond to mouse interaction - OK
    // TODO: make it drag-able - OK
    // TODO: have it have text layed out in it - OK
}


Widget UI_HorizontalLayout(s32 left, s32 top) {
    Widget layout;
    _memzero(&layout, sizeof(Widget));
    layout.layout_kind = LK_HORIZONTAL;
    layout.marg = 0;
    layout.x0 = left;
    layout.y0 = top;
    layout.x = left;
    layout.y = top;
    return layout;
}
Widget UI_VerticalLayout(s32 left, s32 top) {
    Widget layout = UI_HorizontalLayout(left, top);
    layout.layout_kind = LK_VERTICAL;
    layout.marg = 0;
    return layout;
}
Widget UI_VerticalLayoutBelow(Widget layout) {
    Widget vert = UI_VerticalLayout(layout.x, layout.y);
    vert.IncItem(layout.w_max, layout.h_max);
    return vert;
}
void UI_ButtonProperties(s32 *width, s32 *height, s32 *border, Color *col_btn, Color *col_hot) {
    *width = 100;
    *height = 50;
    *border = 4;
    *col_btn = ColorGray(1.0f);
    *col_hot = ColorGray(0.9f);
}
bool UI_Button(Widget *layout, const char *lbl) {
    s32 btn_w;
    s32 btn_h;
    s32 btn_brd;
    Color col_btn;
    Color col_hot;
    UI_ButtonProperties(&btn_w, &btn_h, &btn_brd, &col_btn, &col_hot);

    Color col = col_btn;
    bool clicked = false;
    if (g_mouse->LimsLTWHLastFrame(layout->x, layout->y, btn_w, btn_h)) {
        col = col_hot;
        if (g_mouse->l) {
            btn_brd = 6;
        }
        clicked = g_mouse->ClickedRecently();

        if (clicked == true) {
            printf("click!\n");
        }
    }

    LayoutPanel(layout->x + layout->marg, layout->y + layout->marg, btn_w, btn_h, btn_brd, ColorBlack(), col);
    List<QuadHexaVertex> quads = LayoutText(lbl, layout->x + layout->marg, layout->y + layout->marg, btn_w, btn_h, ColorBlack(), FS_24, TAL_CENTER);

    // vertical align center
    s32 offset_y = btn_h / 2 + GetLineCenterVOffset();
    for (u32 i = 0; i < quads.len; ++i) {
        QuadOffset(quads.lst + i, 0, offset_y);
    }

    layout->IncItem(btn_w, btn_h);
    return clicked;
}
void TestUIButtonMenuManual() {
    printf("TestUIPanel\n");

    MContext *ctx = InitBaselayer();
    GameLoopOne *loop = InitGraphics();

    s32 menu_left = 10;
    s32 menu_top = 10;
    Widget layout_h, layout_v;

    // menu state machine:
    s32 menu_state = -1;
    s32 submenu_state = -1;

    while (loop->GameLoopRunning()) {
        loop->FrameStart2D(ColorGray(0.95f));
        bool menu_clicked = false;

        layout_h = UI_HorizontalLayout(menu_left, menu_top);
        layout_v = UI_VerticalLayoutBelow(layout_h);
        if (UI_Button(&layout_h, "File")) {
            menu_state = menu_state == 0 ? -1 : 0;
            menu_clicked = true;
        }
        if (menu_state == 0) {
            bool b1 = UI_Button(&layout_v, "New");
            bool b2 = UI_Button(&layout_v, "Close");
            bool b3 = UI_Button(&layout_v, "Exit");
            menu_clicked = menu_clicked || b1 || b2 || b3;

            if (b3) {
                loop->Exit();
            }
        }

        layout_v = UI_VerticalLayoutBelow(layout_h);
        if (UI_Button(&layout_h, "Edit")) {
            menu_state = menu_state == 1 ? -1 : 1;
            menu_clicked = true;
        }
        if (menu_state == 1) {
            menu_clicked = menu_clicked || UI_Button(&layout_v, "Clear");
            menu_clicked = menu_clicked || UI_Button(&layout_v, "Config");
        }

        UI_Button(&layout_h, "View");

        layout_v = UI_VerticalLayoutBelow(layout_h);
        if (UI_Button(&layout_h, "Help")) {
            menu_state = menu_state == 2 ? -1 : 2;
            menu_clicked = true;
        }
        if (menu_state == 2) {
            menu_clicked = menu_clicked || UI_Button(&layout_v, "About");
            menu_clicked = menu_clicked || UI_Button(&layout_v, "Help");
        }

        if (g_mouse->ClickedRecently() && menu_clicked == false) {
            menu_state = -1;
        }

        loop->FrameEnd2D();
    }
    loop->Terminate();
}


void TestUILayoutWidgetAPI() {
    printf("TestUILayoutWidgetAPI\n");

    MContext *ctx = InitBaselayer();
    GameLoopOne *loop = InitGraphics();

    UI_Init();

    while (loop->GameLoopRunning()) {
        loop->FrameStart2D(ColorGray(0.95f));

        mlclicked = g_mouse->ClickedRecently();
        mdl = g_mouse->dl;
        ml = g_mouse->l;
        mx = g_mouse->x;
        my = g_mouse->y;
        frameno = loop->frameno;


        if (frameno % 1 == 0) {
            //printf("\ndl: %d fn: %lu\n", g_mouse->dl, frameno);
        }


        // builder code
        bool b = UI_Button("Ok");
        if (b == true) {
            printf("clicked!\n");
        }
        //UI_Button("Cancel");

        UI_FrameEnd(ctx->a_tmp);

        loop->FrameEnd2D();
    }
    loop->Terminate();
}


void Test() {
    //TestRandomPCWithNormals();
    //TestVGROcTree(); // TODO: fix
    //TestQuaternionRotMult(); // TODO: fix
    //TestSlerpAndMat2Quat();
    //TestPointCloudsBoxesAndSceneGraph();
    //TestIndexSetOperations();
    //TestLayoutGlyphQuads();
    //TestBrownianGlyphs();
    //TestUIDragPanel();
    //TestUIButtonMenuManual();
    TestUILayoutWidgetAPI();
}
