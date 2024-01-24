#ifndef __OCTREE_H__
#define __OCTREE_H__


#include "../baselayer.h"
#include "geometry.h"


//
// Octree Stats / runtime and debug info


struct OcTreeStats {
    u32 depth_max;

    u32 max_branches;
    u32 max_leaves;
    u32 actual_branches;
    u32 actual_leaves;

    float box_radius;
    float max_leaf_size;
    float actual_leaf_size;

    float avg_verts_pr_leaf;
    u32 nvertices_in;
    u32 nvertices_out;
    float PctReduced() {
        assert(nvertices_in != 0 && "ensure initialization");
        float pct_reduced = 100.0f * nvertices_out / nvertices_in;
        return pct_reduced;
    }

    void Print() {
        printf("VGR depth: %u\n", depth_max);
        printf("Box radius: %f\n", box_radius);
        printf("Req. leaf size: %f\n", max_leaf_size);
        printf("Act. leaf size: %f\n", actual_leaf_size);
        printf("Branch nodes: %u\n", actual_branches);
        printf("Leaf nodes: %u\n", actual_leaves);
        printf("Vertices in: %u\n", nvertices_in);
        printf("Vertices out: %u\n", nvertices_out);
        printf("Av. vertices pr. cube: %f\n", avg_verts_pr_leaf);
        printf("Reduction pct.: %.2f (%u to %u)\n", 100 - PctReduced(), nvertices_in, nvertices_out);
    }
};
u32 OcTreeStatsSubCubesTotal(u32 depth) {
    u32 ncubes = 0;
    u32 power_of_eight = 1;
    for (u32 i = 1; i <= depth; ++i) {
        power_of_eight *= 8;
        ncubes += power_of_eight;
    }
    return ncubes;
}
u32 OcTreeStatsLeafSize2Depth(float leaf_size, float box_diameter, float *leaf_size_out = NULL) {
    assert(leaf_size_out != NULL);
    if (leaf_size > box_diameter) {
        return 0;
    }

    u32 depth = 1;
    u32 power_of_two = 2;
    while (leaf_size < box_diameter / power_of_two) {
        power_of_two *= 2;
        ++depth;
    }

    *leaf_size_out = box_diameter / power_of_two;
    return depth;
}
OcTreeStats OcTreeStatsInit(float box_radius, float leaf_size_max) {
    OcTreeStats stats;
    float actual_leaf_size;
    u32 depth = OcTreeStatsLeafSize2Depth(leaf_size_max, 2 * box_radius, &actual_leaf_size);
    assert(depth <= 9 && "recommended max depth is 9");
    assert(depth >= 2 && "min depth is 2");

    stats.depth_max = depth;
    u32 max_cubes = OcTreeStatsSubCubesTotal(depth);
    stats.max_branches = OcTreeStatsSubCubesTotal(depth - 1);
    stats.max_leaves = max_cubes - stats.max_branches;
    stats.box_radius = box_radius;
    stats.max_leaf_size = leaf_size_max;
    stats.actual_leaf_size = actual_leaf_size;
    return stats;
}


//
// Voxel Grid Reduce


inline
bool FitsWithinBoxRadius(Vector3f point, float radius) {
    bool result =
        (abs(point.x) <= radius) &&
        (abs( point.y) <= radius) && 
        (abs( point.z) <= radius);
    return result;
}
inline
u8 SubcubeDescend(Vector3f target, Vector3f *center, float *radius) {
    Vector3f relative = target - *center;
    bool neg_x = relative.x < 0;
    bool neg_y = relative.y < 0;
    bool neg_z = relative.z < 0;

    // subcube index 0-7 octets: ---, --+, -+-, -++, +--, +-+, ++-, +++
    u8 subcube_idx = 4*neg_x + 2*neg_y + neg_z;

    // subcube center:
    *radius = 0.5f * (*radius);
    float pm_x = (1 - neg_x * 2) * (*radius);
    float pm_y = (1 - neg_y * 2) * (*radius);
    float pm_z = (1 - neg_z * 2) * (*radius);
    *center = Vector3f {
        center->x + pm_x,
        center->y + pm_y,
        center->z + pm_z
    };

    return subcube_idx;
}
#define VGR_DEBUG 0
struct OcBranch {
    u16 indices[8];
};
struct OcLeaf {
    Vector3f points_sum[8];
    Vector3f normals_sum[8];
    u32 cnt[8];

    #if VGR_DEBUG
    Vector3f center[8];
    float radius[8];
    u8 cube_idx[8];
    #endif
};


static OcBranch _branch_zero;
static OcLeaf _leaf_zero;
struct VoxelGridReduce {
    MArena a;
    OcTreeStats stats;
    Matrix4f box_transform;

    List<OcLeaf> leaves;
    List<OcBranch> branches;

    void AddPoints(List<Vector3f> vertices, List<Vector3f> normals, Matrix4f src_transform) {
        OcLeaf *leaf;
        OcBranch *branch;

        // build the tree
        Vector3f p;
        Vector3f n;
        Vector3f c = Vector3f_Zero();
        float r;
        Matrix4f p2box = TransformGetInverse(box_transform) * src_transform;
        u8 sidx;
        u16 *bidx;
        u16 *lidx;
        for (u32 i = 0; i < vertices.len; ++i) {
            // filter
            p = vertices.lst[i];
            p =  TransformPoint(p2box, p);
            bool keep = FitsWithinBoxRadius(p, stats.box_radius);
            if (keep == false) {
                continue;
            }
            n = normals.lst[i];
            n = TransformDirection(p2box, n);

            // init
            branch = branches.lst;
            r = stats.box_radius;
            c = Vector3f_Zero();

            // d < d_max-1
            for (u32 d = 1; d < stats.depth_max - 1; ++d) {
                sidx = SubcubeDescend(p, &c, &r);
                bidx = branch->indices + sidx;

                if (*bidx == 0) {
                    *bidx = branches.len;
                    branches.Add(_branch_zero);
                }
                branch = branches.lst + *bidx;
            }

            // d == d_max-1
            sidx = SubcubeDescend(p, &c, &r);
            lidx = branch->indices + sidx;
            if (*lidx == 0) {
                assert((u8*) (leaves.lst + leaves.len) == a.mem + a.used && "check memory contiguity");

                *lidx = leaves.len;
                ArenaAlloc(&a, sizeof(OcLeaf));
                leaves.Add(_leaf_zero);
            }
            leaf = leaves.lst + *lidx;

            // d == d_max
            sidx = SubcubeDescend(p, &c, &r);
            #if VGR_DEBUG
            leaf->center[sidx] = c;
            leaf->radius[sidx] = r;
            #endif

            leaf->points_sum[sidx] = leaf->points_sum[sidx] + p;
            leaf->normals_sum[sidx] = leaf->normals_sum[sidx] + n;
            leaf->cnt[sidx] = leaf->cnt[sidx] + 1;
        }
        stats.actual_branches = branches.len;
        stats.actual_leaves = leaves.len;
    }
    void GetPoints(MArena *a_dest, List<Vector3f> *points_dest, List<Vector3f> *normals_dest) {
        assert(points_dest != NULL);
        assert(normals_dest != NULL);

        // points
        u32 npoints_max = leaves.len * 8;
        List<Vector3f> points = InitListOpen<Vector3f>(a_dest, npoints_max);
        Vector3f avg;
        Vector3f sum;
        Vector3f p_world;
        Vector3f n_world;
        u32 cnt = 0;
        float cnt_inv;
        float cnt_sum = 0.0f;
        for (u32 i = 0; i < leaves.len; ++i) {
            OcLeaf leaf = leaves.lst[i];
            for (u32 j = 0; j < 8; ++j) {
                cnt = leaf.cnt[j];
                if (cnt > 0) {
                    cnt_inv = 1.0f / cnt;

                    sum = leaf.points_sum[j];
                    avg = cnt_inv * sum;
                    p_world = TransformPoint(box_transform, avg);
                    points.Add(&p_world);

                    cnt_sum += cnt;
                }
            }
        }
        InitListClose<Vector3f>(a_dest, points.len);
        *points_dest = points;
        stats.nvertices_out = points.len;
        stats.avg_verts_pr_leaf = cnt_sum / stats.nvertices_out;

        // normals
        List<Vector3f> normals = InitListOpen<Vector3f>(a_dest, npoints_max);
        for (u32 i = 0; i < leaves.len; ++i) {
            OcLeaf leaf = leaves.lst[i];
            for (u32 j = 0; j < 8; ++j) {
                cnt = leaf.cnt[j];
                if (cnt > 0) {
                    cnt_inv = 1.0f / cnt;

                    sum = leaf.normals_sum[j];
                    avg = cnt_inv * sum;
                    n_world = TransformDirection(box_transform, avg);
                    normals.Add(n_world);
                }
            }
        }
        InitListClose<Vector3f>(a_dest, normals.len);
        *normals_dest = normals;

    }
};
VoxelGridReduce VoxelGridReduceInit(float leaf_size_max, float box_radius, Matrix4f box_transform) {
    VoxelGridReduce vgr;

    vgr.a = ArenaCreate();
    vgr.box_transform = box_transform;
    vgr.stats = OcTreeStatsInit(box_radius, leaf_size_max);
    vgr.branches = InitList<OcBranch>(&vgr.a, vgr.stats.max_branches / 8);
    vgr.branches.Add(_branch_zero);
    vgr.leaves = InitList<OcLeaf>(&vgr.a, 0);

    return vgr;
}
// TODO: vgr free

#endif
