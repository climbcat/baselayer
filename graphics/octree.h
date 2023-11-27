#ifndef __OCTREE_H__
#define __OCTREE_H__



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

u32 SubCubesTotal(u8 depth) {
    // NOTE: root cube is not counted
    u32 ncubes = 0;
    for (u32 i = 0; i < depth; ++i) {
        ncubes += 8^i;
    }
    return ncubes - 1;
}

u8 SubcubeSelect(Vector3f target, Vector3f cube_center, float cube_radius, Vector3f *subcube_center, float *subcube_radius) {
    assert(subcube_center != NULL && "check out variable");

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

inline
u16 SubCubeAllocBranchBlock(MArena *a_branches, List<VGRBranchBlock> *branch_lst) {
    assert((u8*) (branch_lst->lst + branch_lst->len) == a_branches->mem + a_branches->used && "check memory contiguity");

    u16 result = branch_lst->len;
    ArenaAlloc(a_branches, sizeof(VGRBranchBlock));
    ++branch_lst->len; // TODO: make this a real alloc call (local / safe)

    return result;
}
inline
u16 SubCubeAllocLeafBlock(MArena *a_leaves, List<VGRLeafBlock> *leaf_lst) {
    assert((u8*) (leaf_lst->lst + leaf_lst->len) == a_leaves->mem + a_leaves->used && "check memory contiguity");

    u16 result = leaf_lst->len;
    ArenaAlloc(a_leaves, sizeof(VGRLeafBlock));
    ++leaf_lst->len;

    return result;
}


#endif
