#ifndef __OCTREE_H__
#define __OCTREE_H__


#include "../baselayer.h"
#include "geometry.h"


//
// Octree related things


//
// Voxel Grid Reduce using an octree binning strategy


struct VGRBranchBlock {
    u16 subcube_block_indices[8];
};

struct VGRLeafBlock {
    Vector3f sum[8];
    u32 cnt[8];
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

u8 SubcubeSelect(Vector3f target, Vector3f cube_center, float cube_radius, Vector3f *subcube_center, float *subcube_radius) {
    assert(subcube_center != NULL && subcube_radius != NULL && "non-NULL output variables");

    Vector3f relative = target - cube_center;
    bool neg_x = relative.x < 0;
    bool neg_y = relative.y < 0;
    bool neg_z = relative.z < 0;

    // subcube index 0-7 octets: ---, --+, -+-, -++, +--, +-+, ++-, +++
    u8 subcube_idx = 4*neg_x + 2*neg_y + neg_z;

    // subcube center:
    float pmhalf_x = (1 - neg_x * 2) * 0.5f * cube_radius;
    float pmhalf_y = (1 - neg_y * 2) * 0.5f * cube_radius;
    float pmhalf_z = (1 - neg_z * 2) * 0.5f * cube_radius;
    *subcube_center = Vector3f {
        cube_center.x + pmhalf_x,
        cube_center.y + pmhalf_y,
        cube_center.z + pmhalf_z
    };
    *subcube_radius = 0.5f * cube_radius;

    return subcube_idx;
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
        printf("Voxel Grid Reduce to depth %u:\n", depth_max);
        printf("Box radius: %f and leaf sz: %f, gives actual leaf sz: %f):\n", box_radius, max_leaf_size, actual_leaf_size);
        printf("Built %u branch node blocks and %u leaf node blocks\n", actual_branches, actual_leaves);
        printf("Inputs: %u, outputs: %u vertices (avg. %f verts pr. leaf cube)\n", nvertices_in, nvertices_out, avg_verts_pr_leaf);
        printf("Reduced by: %.2f pct (from %u to %u)\n", 100 - PctReduced(), nvertices_in, nvertices_out);
    }
};

inline
bool FitsWithinBox(Vector3f point, Vector3f center, float radius) {
    bool result = (abs( point.x - center.x ) <= radius) &&
        (abs( point.y - center.y ) <= radius) && 
        (abs( point.z - center.z ) <= radius);
    return result;
}
inline
bool FitsWithinBoxRadius(Vector3f point, float radius) {
    bool result =
        (abs(point.x) <= radius) &&
        (abs( point.y) <= radius) && 
        (abs( point.z) <= radius);
    return result;
}


u32 MaxLeafSize2Depth(float leaf_size_max, float box_radius, float *leaf_size_out = NULL) {
    u32 depth = 0;
    u32 power_of_two = 1;
    if (leaf_size_max > box_radius / power_of_two) {
        return depth;
    }

    depth = 1;
    power_of_two = 2;
    while (leaf_size_max < box_radius / power_of_two) {
        power_of_two *= 2;
        ++depth;
    }

    if (leaf_size_out != NULL) {
        *leaf_size_out = box_radius / power_of_two;
    }
    return depth;
}
float Depth2LeafSize(u32 depth, float box_radius) {
    float leaf_sz = box_radius;
    for (u32 i = 2; i <= depth; ++i) {
        leaf_sz = leaf_sz / 2;
    }
    return leaf_sz;
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
    VGRTreeStats *stats_out = NULL)
{
    assert(dest != NULL);
    assert(box_radius > 0);
    assert(leaf_size_max > 0);
    assert(tmp != NULL);
    assert(vertices.lst != NULL);
    assert(dest != NULL);


    // setup
    VGRTreeStats stats;
    {
        // determine depth
        u32 depth = 2;
        u32 power_of_two = 2 * 2;
        while (leaf_size_max < box_radius / power_of_two) {
            power_of_two *= 2;
            ++depth;
        }
        float actual_leaf_size = box_radius / power_of_two;
        assert(depth <= 8 && "recommended max depth is 8");
        assert(depth >= 2 && "min depth is 2");

        // tiny test
        u32 depth2 = MaxLeafSize2Depth(leaf_size_max, box_radius);
        float leaf_sz_2 = Depth2LeafSize(depth, box_radius);
        assert(depth == depth2);
        assert(leaf_sz_2 = actual_leaf_size);

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
        assert(stats.max_leaves / 8 <= 65535 && "block index address space max exceeded");
    }

    // reserve branch memory, assign level 1 branches:
    List<VGRBranchBlock> branch_lst = InitList<VGRBranchBlock>(tmp, stats.max_branches / 8);
    static VGRBranchBlock branch_zero;
    branch_lst.Add(&branch_zero);
    VGRBranchBlock *branch_block = branch_lst.lst;

    // open-ended size for leaf blocks:
    List<VGRLeafBlock> leaf_lst = InitList<VGRLeafBlock>(tmp, 0);
    static VGRLeafBlock leaf_zero;
    VGRLeafBlock *leaf_block;

    // build the tree
    Vector3f p;
    Vector3f c = Vector3f_Zero();
    float r;
    float sign = 1.0f;
    if (flip_y == true) {
        sign = -1.0f;
    }
    Matrix4f p2box = TransformGetInverse(box_transform) * src_transform;
    for (u32 i = 0; i < vertices.len; ++i) {
        // Db print:
        //printf("%u: ", i);

        // get vertex, transform, flip and box filter.
        // NOTE: Everything happens in box coords until the resulting VGR vertex is transformed back to world and added to the box
        p = sign * TransformPoint(p2box, vertices.lst[i]);
        bool keep = FitsWithinBoxRadius(p, stats.box_radius);
        if (keep == false) {
            continue;
        }

        // init
        branch_block = branch_lst.lst;
        r = stats.box_radius;

        // descend
        for (u32 d = 1; d < stats.depth_max - 1; ++d) {
            u8 sub_idx = SubcubeSelect(p, c, r, &c, &r);
            u16 *block_idx = &branch_block->subcube_block_indices[sub_idx];

            if (*block_idx == 0) {
                // unassigned level d+1 branch
                // TODO: inline Add zero'd block to branch_lst
                *block_idx = branch_lst.len;
                branch_lst.Add(&branch_zero);
            }
            branch_block = branch_lst.lst + *block_idx;
        }

        // almost there, d == d_max - 1
        u8 sub_idx = SubcubeSelect(p, c, r, &c, &r);
        u16 *leaf_block_idx = &branch_block->subcube_block_indices[sub_idx];
        if (*leaf_block_idx == 0) {
            // unassigned level d_max leaf
            assert((u8*) (leaf_lst.lst + leaf_lst.len) == tmp->mem + tmp->used && "check memory contiguity");

            *leaf_block_idx = leaf_lst.len;
            ArenaAlloc(tmp, sizeof(VGRLeafBlock));
            leaf_lst.Add(&leaf_zero);
        }
        leaf_block = leaf_lst.lst + *leaf_block_idx;

        // finally at d == depth_max: select leaf in leaf_block
        sub_idx = SubcubeSelect(p, c, r, &c, &r);
        // Db print:
        //printf("lbi/scube: %u %u \n", *leaf_block_idx, sub_idx);

        Vector3f *sum = &leaf_block->sum[sub_idx];
        *sum = *sum + p;
        u32 *cnt = &leaf_block->cnt[sub_idx];
        *cnt = *cnt + 1;
    }
    stats.actual_branches = branch_lst.len;
    stats.actual_leaves = leaf_lst.len;

    // walk the tree
    List<Vector3f> result = { dest, 0 };
    Vector3f avg;
    Vector3f sum;
    Vector3f p_world;
    u32 cnt = 0;
    float cnt_inv;
    float cnt_sum = 0.0f;
    for (u32 i = 0; i < leaf_lst.len; ++i) {
        VGRLeafBlock lb = leaf_lst.lst[i];
        for (u32 j = 0; j < 8; ++j) {
            cnt = lb.cnt[j];
            if (cnt > 0) {
                sum = lb.sum[j];
                cnt_inv = 1.0f / cnt;
                avg = cnt_inv * sum;

                // Here, we go back to world coords after having worked in box coords all along
                p_world = TransformPoint(box_transform, avg);
                result.Add(&p_world);

                cnt_sum += cnt;
            }
        }
    }

    stats.nvertices_out = result.len;
    stats.avg_verts_pr_leaf = cnt_sum / stats.nvertices_out;
    if (stats_out != NULL) {
        *stats_out = stats;
    }

    return result;
}


#endif
