#ifndef __INDICES_H__
#define __INDICES_H__


#include "geometry.h"


//
//  Preserves the order of extraction indices.
//  Preserves the number of extraction indices.
//  Extraction index set may have duplicates
//  Preserves the order of values.
//  Values will not have duplicates.
//
u32 IndicesExtract(
    MArena *a_dest,
    MArena *a_tmp,
    List<Vector3f> *values_dest,
    List<u32> *indices_dest,
    List<Vector3f> values,
    List<u32> indices)
{
    // output indices size is given
    *indices_dest = InitList<u32>(a_dest, indices.len);

    // init indirection indices
    List<u32> indirection = InitList<u32>(a_tmp, indices.len);
    for (u32 i = 0; i < indirection.len; ++i) {
        indirection.Add(-1);
    }

    // mark indices for addition
    u32 cp_cnt = 0;
    for (u32 i = 0; i < indices.len; ++i) {
        u32 idx = indices.lst[i];
        if (indirection.lst[idx] == -1) {
            ++cp_cnt;
            indirection.lst[idx] = idx;
        }
    }

    // compile values, record index shifts
    *values_dest = InitList<Vector3f>(a_dest, cp_cnt);
    u32 cp_acc = 0;
    for (u32 i = 0; i < indirection.len; ++i) {
        u32 idx_indir = indirection.lst[i];

        if (idx_indir != -1) {
            ++cp_acc;
            assert(idx_indir == i);

            indirection.lst[i] = i - cp_acc;
            values_dest->Add(values.lst[idx_indir]);
        }
    }
    assert(cp_acc == cp_cnt && "sanity check");

    // recompile indices that hit marked points
    for (u32 i = 0; i < indices.len; ++i) {
        u32 indir = indirection.lst[indices.lst[i]];
        assert(indir != -1 && "sanity check");

        indices_dest->Add(indir);
    }

    // return number of extracted values
    return values_dest->len;
}


//
//  Index sets may have duplicates.
//  Result allocated on tmp arena.
//
List<u32> IndicesRemove(
    MArena *a_dest,
    MArena *a_tmp,
    List<Vector3f> *values_dest,
    List<u32> *indices_dest,
    List<Vector3f> values,
    List<u32> indices,
    List<u32> indices_rm)
{
    // build indirection array and get value hit-count / removal-count
    List<u32> indirection = InitList<u32>(a_tmp, values.len);
    for (u32 i = 0; i < values.len; ++i) {
        indirection.Add(i);
    }
    u32 rm_cnt = 0;
    for (u32 i = 0; i < indices_rm.len; ++i) {
        u32 idx = indices.lst[i];

        if (indirection.lst[idx] != -1) {
            ++rm_cnt;
            indirection.lst[idx] = -1;
        }
    }

    // compile values, record shifted locations in the indirections list
    *values_dest = InitList<Vector3f>(a_dest, values.len - rm_cnt);
    u32 rm_acc = 0;
    for (u32 i = 0; i < indirection.len; ++i) {
        u32 idx = indirection.lst[i];

        if (idx == -1) {
            ++rm_acc;
        }
        else {
            assert(idx == i);

            indirection.lst[i] = i - rm_acc;
            values_dest->lst[i - rm_acc] = values.lst[idx];
        }
    }
    assert(rm_acc == rm_cnt && "sanity check");
    indirection.len -= rm_cnt;

    // build filtered index set
    *indices_dest = InitList<u32>(a_dest, 0);
    for (u32 i = 0; i < indices.len; ++i) {
        u32 idx_indir = indirection.lst[indices.lst[i]];

        if (idx_indir != -1) {
            ArenaAlloc(a_dest, sizeof(u32));
            indices_dest->Add(idx_indir);
        }
    }

    // NOTE: result allocated on tmp arena
    return indirection;
}


//
//  Appended indices may have negative values; corresponding to a stitch-type operation.
//
void IndicesAppend(
    MArena *a_dest,
    List<Vector3f> *values_dest,
    List<u32> *indices_dest,
    List<Vector3f> values,
    List<u32> indices,
    List<Vector3f> values_append,
    List<s32> indices_append)
{
    *values_dest = InitList<Vector3f>(a_dest, values.len + values_append.len);
    *indices_dest = InitList<u32>(a_dest, indices.len + indices_append.len);

    // copy values naiively
    _memcpy(values_dest->lst, values.lst, values.len * sizeof(Vector3f));
    values_dest->len = values.len;
    _memcpy(values_dest->lst + values_dest->len, values_append.lst, values_append.len * sizeof(Vector3f));
    values_dest->len += values_append.len;
    // copy base indices
    _memcpy(indices_dest->lst, indices.lst, indices.len * sizeof(u32));
    indices_dest->len = indices.len;

    // copy shifted append indices
    u32 shift = values.len;
    for (u32 i = 0; i < indices_append.len; ++i) {
        u32 append = indices_append.lst[i];
        assert(append + shift >= 0 && "check producing actual index");
        u32 idx = append + shift;
        indices_dest->Add(idx);
    }
}


// TODO: various functions for use with an indirection set, e.g.
//      - select from a values array
//      - apply indirection to index array
//      - select from an index array


#endif
