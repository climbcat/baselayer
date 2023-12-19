#ifndef __OCTREE_H__
#define __OCTREE_H__


#include "../baselayer.h"
#include "geometry.h"


//
// Octree related things


//
// Voxel Grid Reduce using an octree binning strategy

#define VGR_DEBUG

struct OcBranch {
    u16 indices[8];
};
struct OcLeaf {
    Vector3f sum[8];
    u32 cnt[8];

    #ifdef VGR_DEBUG
    Vector3f center;
    float radius;
    u8 cube_idx;
    #endif
};


u32 SubCubesTotal(u32 depth) {
    // NOTE: root cube is not counted
    u32 ncubes = 0;
    u32 power_of_eight = 1;
    for (u32 i = 1; i <= depth; ++i) {
        power_of_eight *= 8;
        ncubes += power_of_eight;
    }
    return ncubes;
}
struct VGRTreeStats {
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
        printf("Act. leaf size: %f:\n", actual_leaf_size);
        printf("Branch nodes: %u\n", actual_branches);
        printf("Leaf nodes: %u\n", actual_leaves);
        printf("Vertices in: %u\n", nvertices_in);
        printf("Vertices out: %u\n", nvertices_out);
        printf("Av. vertices pr. cube: %f\n", avg_verts_pr_leaf);
        printf("Reduction pct.: %.2f (%u to %u)\n", 100 - PctReduced(), nvertices_in, nvertices_out);
    }
};

inline
bool FitsWithinBoxRadius(Vector3f point, float radius) {
    bool result =
        (abs(point.x) <= radius) &&
        (abs( point.y) <= radius) && 
        (abs( point.z) <= radius);
    return result;
}


u32 LeafSize2Depth(float leaf_size, float box_diameter, float *leaf_size_out = NULL) {
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
float Depth2LeafSize(u32 depth, float box_radius) {
    float leaf_sz = box_radius;
    for (u32 i = 2; i <= depth; ++i) {
        leaf_sz = leaf_sz / 2;
    }
    return leaf_sz;
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
List<Vector3f> VoxelGridReduce(
    List<Vector3f> vertices,
    MArena *tmp,
    float box_radius,
    float leaf_size_max,
    Matrix4f box_transform,
    Matrix4f src_transform,
    Vector3f *dest, // you have to reserve memory at this location
    bool flip_y = false,
    VGRTreeStats *stats_out = NULL,
    List<OcLeaf> *leaves_out = NULL,
    List<OcBranch> *branches_out = NULL)
{
    assert(tmp != NULL);
    assert(dest != NULL);
    assert(vertices.lst != NULL);
    assert(box_radius > 0);
    assert(leaf_size_max > 0);


    // setup
    VGRTreeStats stats;
    {
        // determine depth
        float actual_leaf_size;
        u32 depth = LeafSize2Depth(leaf_size_max, 2 * box_radius, &actual_leaf_size);
        assert(depth <= 9 && "recommended max depth is 9");
        assert(depth >= 2 && "min depth is 2");

        // determine reserve sizes, record params
        stats.depth_max = depth;
        u32 max_cubes = SubCubesTotal(depth);
        stats.max_branches = SubCubesTotal(depth - 1);
        stats.max_leaves = max_cubes - stats.max_branches;
        stats.box_radius = box_radius;
        stats.max_leaf_size = leaf_size_max;
        stats.actual_leaf_size = actual_leaf_size;
        stats.nvertices_in = vertices.len;

        // TODO: try and run the alg with max memory, assuming that max occupancy very rarely happens
        //assert(stats.max_leaves / 8 <= 65535 && "block index address space max exceeded");
    }

    // reserve branch memory
    List<OcBranch> branches = InitList<OcBranch>(tmp, stats.max_branches / 8);
    static OcBranch branch_zero;

    // assign level 1 branches:
    branches.Add(branch_zero);
    OcBranch *branch = branches.lst;

    // open-ended size for leaf blocks:
    List<OcLeaf> leaves = InitList<OcLeaf>(tmp, 0);
    static OcLeaf leaf_zero;
    OcLeaf *leaf;

    // build the tree
    Vector3f p;
    Vector3f c = Vector3f_Zero();
    float r;
    float sign_y = 1.0f;
    if (flip_y == true) {
        sign_y = -1.0f;
    }
    Matrix4f p2box = TransformGetInverse(box_transform) * src_transform;
    u8 sidx;
    u16 *bidx;
    u16 *lidx;
    for (u32 i = 0; i < vertices.len; ++i) {
        // filter
        p = vertices.lst[i];
        p.y *= sign_y;
        p =  TransformPoint(p2box, p);
        bool keep = FitsWithinBoxRadius(p, stats.box_radius);
        if (keep == false) {
            continue;
        }

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
                branches.Add(&branch_zero);
            }
            branch = branches.lst + *bidx;
        }

        // d == d_max-1
        sidx = SubcubeDescend(p, &c, &r);
        lidx = branch->indices + sidx;
        if (*lidx == 0) {
            assert((u8*) (leaves.lst + leaves.len) == tmp->mem + tmp->used && "check memory contiguity");

            *lidx = leaves.len;
            ArenaAlloc(tmp, sizeof(OcLeaf));
            leaves.Add(&leaf_zero);
        }
        leaf = leaves.lst + *lidx;

        // d == d_max
        sidx = SubcubeDescend(p, &c, &r);
        #ifdef VGR_DEBUG
        leaf->cube_idx = sidx;
        leaf->center = c;
        leaf->radius = r;
        #endif

        leaf->sum[sidx] = leaf->sum[sidx] + p;
        leaf->cnt[sidx] = leaf->cnt[sidx] + 1;
    }
    stats.actual_branches = branches.len;
    stats.actual_leaves = leaves.len;


    // leaf iteration, generate averages
    List<Vector3f> points = { dest, 0 };
    Vector3f avg;
    Vector3f sum;
    Vector3f p_world;
    u32 cnt = 0;
    float cnt_inv;
    float cnt_sum = 0.0f;
    for (u32 i = 0; i < leaves.len; ++i) {
        OcLeaf leaf = leaves.lst[i];
        for (u32 j = 0; j < 8; ++j) {
            cnt = leaf.cnt[j];
            if (cnt > 0) {
                sum = leaf.sum[j];
                cnt_inv = 1.0f / cnt;
                avg = cnt_inv * sum;

                // Here, we go back to world coords after having worked in box coords all along
                p_world = TransformPoint(box_transform, avg);
                points.Add(&p_world);

                cnt_sum += cnt;
            }
        }
    }

    // record stats
    stats.nvertices_out = points.len;
    stats.avg_verts_pr_leaf = cnt_sum / stats.nvertices_out;
    if (stats_out != NULL) {
        *stats_out = stats;
    }
    // assign output (debug) vars
    if (leaves_out) *leaves_out = leaves;
    if (branches_out) *branches_out = branches;

    return points;
}


#endif
