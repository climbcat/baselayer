#ifndef __INDICES_H__
#define __INDICES_H__


//
//  Preserves the order of indices.
//  Preserves the number of indices.
//  Index sets may have duplicates
//  Preserves the order of values.
//  Values will not have duplicates.
//


//
//  Append-indices may be negative, enabling stitching-like operations.
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

    _memcpy(values_dest->lst, values.lst, values.len * sizeof(Vector3f));
    values_dest->len = values.len;
    _memcpy(values_dest->lst + values_dest->len, values_append.lst, values_append.len * sizeof(Vector3f));
    values_dest->len += values_append.len;
    _memcpy(indices_dest->lst, indices.lst, indices.len * sizeof(u32));
    indices_dest->len = indices.len;

    // shifted and copy append-indices
    u32 shift = values.len;
    for (u32 i = 0; i < indices_append.len; ++i) {
        u32 append = indices_append.lst[i];
        assert(append + shift >= 0 && "check producing actual index");
        u32 idx = append + shift;
        indices_dest->Add(idx);
    }
}


//
//  Indirection-set based functions / building blocks for extract/remove.
//  The indirection set is an index set, of the same length as the values list,
//  (or the largest index into the values array).
//
//  The indirection set is used to re-code index arrays. Indirection sets can mark and shift
//  indices into a new values array, a selection of the original.
//


List<u32> IndicesMarkPositive(MArena *a_dest, u32 vals_len, List<u32> idxs_pos) {
    List<u32> indirection = InitList<u32>(a_dest, vals_len);
    indirection.len = vals_len;
    for (u32 i = 0; i < indirection.len; ++i) {
        indirection.lst[i] = -1;
    }

    u32 idx;
    for (u32 i = 0; i < idxs_pos.len; ++i) {
        idx = idxs_pos.lst[i];
        indirection.lst[idx] = idx;
    }
    return indirection;
}


List<u32> IndicesMarkNegative(MArena *a_dest, u32 vals_len, List<u32> idxs_neg) {
    List<u32> indirection = InitList<u32>(a_dest, vals_len);
    indirection.len = vals_len;
    for (u32 i = 0; i < indirection.len; ++i) {
        indirection.lst[i] = i;
    }

    u32 idx;
    for (u32 i = 0; i < idxs_neg.len; ++i) {
        idx = idxs_neg.lst[i];
        indirection.lst[idx] = -1;
    }
    return indirection;
}


List<u32> IndicesMarkPositiveNegative(MArena *a_dest, u32 vals_len, List<u32> idxs_pos, List<u32> idxs_neg) {
    List<u32> indirection = InitList<u32>(a_dest, vals_len);
    indirection.len = vals_len;
    for (u32 i = 0; i < indirection.len; ++i) {
        indirection.lst[i] = -1;
    }

    u32 idx;
    for (u32 i = 0; i < idxs_pos.len; ++i) {
        idx = idxs_pos.lst[i];
        indirection.lst[idx] = idx;
    }

    for (u32 i = 0; i < idxs_neg.len; ++i) {
        idx = idxs_neg.lst[i];
        indirection.lst[idx] = -1;
    }
    return indirection;
}


void IndicesShiftDownIndirectionList(List<u32> indirection) {
    u32 idx;
    u32 acc = 0;
    for (u32 i = 0; i < indirection.len; ++i) {
        idx = indirection.lst[i];
        if (idx == -1) {
            ++acc;
        }
        else {
            assert(i == idx && "");
            indirection.lst[i] = i - acc;
        }
    }
}


void IndicesIndirectInline(List<u32> idxs, List<u32> indirection) {
    u32 idx;
    for (u32 i = 0; i < idxs.len; ++i) {
        idx = idxs.lst[i];
        assert(idx < indirection.len && "");

        idxs.lst[i] = indirection.lst[idx];
    }
}


List<u32> IndicesIndirect(MArena *a_dest, List<u32> idxs, List<u32> indirection) {
    List<u32> idxs_out = InitList<u32>(a_dest, 0);
    u32 idx;
    for (u32 i = 0; i < idxs.len; ++i) {
        idx = idxs.lst[i];

        assert(idx < indirection.len && "");
        ArenaAlloc(a_dest, sizeof(u32));
        idxs_out.Add(indirection.lst[idx]);
    }
    return idxs_out;
}


List<u32> IndicesIndirectOrRemove(MArena *a_dest, List<u32> idxs, List<u32> indirection) {
    List<u32> idxs_out = InitList<u32>(a_dest, 0);
    u32 idx;
    u32 indir;
    for (u32 i = 0; i < idxs.len; ++i) {
        idx = idxs.lst[i];
        assert(idx < indirection.len && "");

        indir = indirection.lst[idx];
        if (indir != -1) {
            ArenaAlloc(a_dest, sizeof(u32));
            idxs_out.Add(indir);
        }
    }
    return idxs_out;
}


template<class T>
List<T> IndicesSelectValues(MArena *a_dest, List<T> values, List<u32> indirection) {
    List<T> seln = InitList<T>(a_dest, 0);
    T val;
    u32 idx;
    for (u32 i = 0; i < indirection.len; ++i) {
        idx = indirection.lst[i];

        if (idx != -1) {
            val = values.lst[i];
            ArenaAlloc(a_dest, sizeof(T));
            seln.Add(val);
        }
    }
    return seln;
}


//
//  Wrappers
//


template<class T>
List<u32> IndicesExtract(MArena *a_dest, MArena *a_indir, List<T> values, List<u32> idxs, List<T> *values_out, List<u32> *idxs_out) {
    List<u32> indirection = IndicesMarkPositive(a_indir, values.len, idxs);
    IndicesShiftDownIndirectionList(indirection);

    *values_out = IndicesSelectValues<T>(a_dest, values, indirection);
    *idxs_out = IndicesIndirect(a_dest, idxs, indirection);

    return indirection;
}
template<class T>
List<u32> IndicesExtractVals(MArena *a_dest, MArena *a_indir, List<T> values, List<u32> idxs, List<T> *values_out) {
    List<u32> indirection = IndicesMarkPositive(a_indir, values.len, idxs);
    IndicesShiftDownIndirectionList(indirection);

    *values_out = IndicesSelectValues<T>(a_dest, values, indirection);

    return indirection;
}


template<class T>
List<u32> IndicesRemove(MArena *a_dest, MArena *a_indir, List<T> values, List<u32> idxs, List<u32> idxs_rm, List<T> *values_out, List<u32> *idxs_out) {
    List<u32> indirection = IndicesMarkPositiveNegative(a_indir, values.len, idxs, idxs_rm);
    IndicesShiftDownIndirectionList(indirection);

    *values_out = IndicesSelectValues<T>(a_dest, values, indirection);
    *idxs_out = IndicesIndirectOrRemove(a_dest, idxs, indirection);

    return indirection;
}

List<u32> IndicesRemove(MArena *a_dest, MArena *a_indir, u32 vals_len, List<u32> idxs, List<u32> idxs_rm, List<u32> *idxs_out) {
    List<u32> indirection = IndicesMarkPositiveNegative(a_indir, vals_len, idxs, idxs_rm);
    IndicesShiftDownIndirectionList(indirection);

    *idxs_out = IndicesIndirectOrRemove(a_dest, idxs, indirection);

    return indirection;
}


#endif
