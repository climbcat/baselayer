
#include "../baselayer.h"
#include "geometry.h"
#include "octree.h"
#include "ui.h"
#include "swrender.h"
#include "gameloop.h"


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


void TestRandomPCsRotatingBoxes() {
    printf("TestRandomPCsRotatingBoxes\n");

    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();

    Entity *axes = EntityCoordAxes(es, r);
    Entity *box = EntityAABox(es, { 0.3f, 0.0f, 0.7f }, 0.2f, r);
    Entity *box2 = EntityAABox(es, { 0.3f, 0.0f, -0.7f }, 0.2f, r);
    Entity *box3 = EntityAABox(es, { -0.7f, 0.0f, 0.0f }, 0.2f, r);

    box->tpe = ET_LINES_ROT;
    box2->tpe = ET_LINES_ROT;
    box3->tpe = ET_LINES_ROT;

    // point clouds 
    MArena _a_pointclouds = ArenaCreate();
    MArena *a_pcs = &_a_pointclouds;
    List<Vector3f> points_1 = CreateRandomPointCloud(a_pcs, 90, { 0.0f, 0.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
    Entity *pc_1 = EntityPoints(es, &points_1);
    pc_1->color = { RGBA_GREEN };
    List<Vector3f> points_2 = CreateRandomPointCloud(a_pcs, 300, { 0.0f, 0.0f, -1.f }, { 4.0f, 4.0f, 2.0f });
    Entity *pc_2 = EntityPoints(es, &points_2);
    pc_2->color = { RGBA_BLUE };
    List<Vector3f> points_3 = CreateRandomPointCloud(a_pcs, 300, { -1.0f, -1.0f, -1.f }, { 2.0f, 2.0f, 2.0f });
    Entity *pc_3 = EntityPoints(es, &points_3);
    pc_3->color = { RGBA_RED };

    EntitySystemPrint(es);
    StreamPrint(pc_1->entity_stream, (char*) "random point clouds");

    loop->JustRun(es);
}


void TestRandomPCWithNormals() {
    printf("TestRandomPCWithNormals\n");

    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();
    Entity *axes = EntityCoordAxes(es, r);
    
    MArena _a_pointclouds = ArenaCreate();
    MArena *a_pcs = &_a_pointclouds;

    // point clouds 
    List<Vector3f> points = CreateRandomPointCloud(a_pcs, 90, { 0.0f, 0.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
    List<Vector3f> normals = CreateRandomPointCloud(a_pcs, 90, { 0.1f, 0.1f, 0.1f }, { 0.05f, 0.05f, 0.05f });

    Entity *pc = EntityPoints(es, &points);
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
    float leaf_size = rootcube_radius / 2.0 / 2.0;
    u32 nvertices_src = 10000;
    bool display_boxes = false;
    Color color_in { RGBA_BLUE };
    Color color_out { RGBA_RED };


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

    List<Vector3f> dest = InitList<Vector3f>(a_dest, nvertices_src);
    Matrix4f box_transform = Matrix4f_Identity();
    Matrix4f src_transform = Matrix4f_Identity();

    // run the vgr
    VGRTreeStats stats;
    List<OcLeaf> leaf_blocks_out;
    List<OcBranch> branch_blocks_out;
    dest = VoxelGridReduce(src, a_tmp, rootcube_radius, leaf_size, box_transform, src_transform, dest.lst, false, &stats, &leaf_blocks_out, &branch_blocks_out);

    // visualize
    GameLoopOne *loop = InitGameLoopOne();
    SwRenderer *r = loop->GetRenderer();
    EntitySystem *es = InitEntitySystem();
    Entity *axes = EntityCoordAxes(es, r);

    Entity *src_pc = EntityPoints(es, &src);
    src_pc->transform = src_transform;
    src_pc->tpe = ET_POINTCLOUD;
    src_pc->color = color_in;

    Entity *dest_pc = EntityPoints(es, &dest);
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
    stats.Print();

    loop->JustRun(es);
}


void Test() {
    //TestRandomPCsRotatingBoxes();
    //TestRandomPCWithNormals();
    TestVGROcTree();
}
