#ifndef __OCTREE_H__
#define __OCTREE_H__



//
// Octree related things


//
// Voxel Grid Reduce using an octree binning strategy


struct VGRBranch {
    u16 subcube_block_index = 0; // subcube_block_index; counting blocks of 8 branches or 8 leafs
};

struct VGRLeaf {
    Vector3f sum { 0, 0, 0 }; // vector sum
    u32 cnt = 0; // count vectors in sum
};

u32 SubCubesTotal(u8 depth) {
    // NOTE: root cube is not counted
    u32 ncubes = 0;
    for (u32 i = 0; i < depth; ++i) {
        ncubes += 8^i;
    }
    return ncubes - 1;
}

u8 SubcubeSelect(Vector3f target, Vector3f cube_center, float cube_radius, Vector3f *subcube_center) {
    assert(subcube_center != NULL && "check out variable");

    Vector3f rel = target - cube_center;
    u8 neg_x = rel.x >= 0;
    u8 neg_y = rel.y >= 0;
    u8 neg_z = rel.z >= 0;

    // subcube index 0-7 octets: ---, --+, -+-, -++, +--, +-+, ++-, +++
    u8 subcube_idx = 4*neg_x + 2*neg_y + neg_z;

    // subcube center:
    s8 pmhalf_x = (1 - neg_x * 2) * 0.5f * cube_radius;
    s8 pmhalf_y = (1 - neg_y * 2) * 0.5f * cube_radius;
    s8 pmhalf_z = (1 - neg_z * 2) * 0.5f * cube_radius;
    *subcube_center = Vector3f {
        cube_center.x + pmhalf_x,
        cube_center.y + pmhalf_y,
        cube_center.z + pmhalf_z
    };

    return subcube_idx;
}

inline
u16 SubCubeAllocBranches(VGRBranch current, List<VGRBranch> *branches) {
    // returns a new block index
    assert(branches->len % 8 == 0 && "ensure size of whole blocks have been allocated so far");
    u16 result = branches->len / 8;
    branches->len += 8; // TODO: make this a real alloc call (local / safe)
    return result;
}
inline
u16 SubCubeAllocLeaves(VGRBranch current, List<VGRLeaf> *leaves) {
    assert(leaves->len % 8 == 0 && "ensure size of whole blocks have been allocated so far");
    u16 result = leaves->len;
    leaves->len += 8; // TODO: makes this a real alloc call (local / safe)
    return result;
}


#endif
