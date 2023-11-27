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
void OcTreeTestRandomPoints() {
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

#endif
