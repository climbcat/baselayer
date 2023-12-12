
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
    Entity *pc_1 = EntityPoints(es, points_1);
    pc_1->color = { RGBA_GREEN };
    List<Vector3f> points_2 = CreateRandomPointCloud(a_pcs, 300, { 0.0f, 0.0f, -1.f }, { 4.0f, 4.0f, 2.0f });
    Entity *pc_2 = EntityPoints(es, points_2);
    pc_2->color = { RGBA_BLUE };
    List<Vector3f> points_3 = CreateRandomPointCloud(a_pcs, 300, { -1.0f, -1.0f, -1.f }, { 2.0f, 2.0f, 2.0f });
    Entity *pc_3 = EntityPoints(es, points_3);
    pc_3->color = { RGBA_RED };

    EntitySystemPrint(es);
    StreamPrint(pc_1->entity_stream, (char*) "random point clouds");

    loop->JustRun(es);
}


void TestVGROcTree() {
    printf("TestVGROcTree\n");

    // filtering box / octree location
    float rootcube_radius = 0.2;
    float leaf_size_max = rootcube_radius / (2 * 2 * 2 * 2 * 2 * 1.9);

    // src/dst storage
    MArena _a_tmp = ArenaCreate(); // vertices & branch list location
    MArena *a_tmp = &_a_tmp;
    MArena _a_dest = ArenaCreate(); // dest storage
    MArena *a_dest = &_a_dest;

    // source random point cloud
    u32 nvertices = 190000;
    Vector3f rootcube_center { 0, 0, 0 };
    float pc_radius = 0.2;
    List<Vector3f> vertices = InitList<Vector3f>(a_tmp, nvertices);
    RandInit();
    for (u32 i = 0; i < nvertices; ++i) {
        Vector3f v {
            rootcube_center.x - pc_radius + 2*pc_radius*Rand01_f32(),
            rootcube_center.y - pc_radius + 2*pc_radius*Rand01_f32(),
            rootcube_center.y - pc_radius + 2*pc_radius*Rand01_f32(),
        };
        vertices.Add(&v);
    }

    // dest list
    List<Vector3f> dest = InitList<Vector3f>(a_dest, nvertices);

    // transforms
    Matrix4f box_transform = TransformBuildRotateX(15*deg2rad);
    Matrix4f src_transform = TransformBuildRotateY(45*deg2rad);

    VGRTreeStats stats;
    dest = VoxelGridReduce(vertices, a_tmp, rootcube_radius, leaf_size_max, box_transform, src_transform, dest.lst, false, &stats);

    stats.Print();
    printf("\n");
}


void Test() {
    TestRandomPCsRotatingBoxes();
    //TestVGROcTreeInitial();
    //TestVGROcTree();
}
