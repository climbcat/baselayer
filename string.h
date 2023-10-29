#ifndef __STRING_H__
#define __STRING_H__

#include <cstdio>
#include <cassert>


// NOTE: currently no "string list header" struct, which means that strings and str lst are
//       treated a bit differently: Strings are passed as a struct, but str lists as a pointer.
// NOTE: the string list is in fact an LList1, maybe use the functions from base.c


//
// Str and StrLst


struct Str {
    char *str = NULL;
    u32 len = 0;
};

struct StrLst {
    StrLst *next = NULL;
    char *str = NULL;
    u32 len = 0;
};

Str StrLiteral(MArena *a, const char *lit) {
    // TODO: make into StrPut
    Str s;
    s.len = 0;
    while (*(lit + s.len) != '\0') {
        ++s.len;
    }
    s.str = (char*) ArenaAlloc(a, s.len);
    _memcpy(s.str, lit, s.len);

    return s;
}
// TODO: get variadic
void StrPrint(const char *format, Str s) {
    //u32 format_str_max_len = 255;
    //char buff[s.len + format_str_max_len];
    //sprintf(buff, "%.*s", s.len, s.str);
    //printf(format, buff);
}
inline void StrPrint(Str s) {
    printf("%.*s", s.len, s.str);
}
inline void StrPrint(StrLst s) {
    printf("%.*s", s.len, s.str);
}
inline void StrPrint(Str *s) {
    printf("%.*s", s->len, s->str);
}
inline void StrPrint(StrLst *s) {
    printf("%.*s", s->len, s->str);
}
bool StrEqual(Str a, Str b) {
    u32 i = 0;
    u32 len = MinU32(a.len, b.len);
    while (i < len) {
        if (a.str[i] != b.str[i]) {
            return false;
        }
        ++i;
    }

    return a.len == b.len;
}
Str StrCat(MArena *arena, Str a, Str b) {
    Str cat;
    cat.len = a.len + b.len;
    cat.str = (char*) ArenaAlloc(arena, cat.len);
    _memcpy(cat.str, a.str, a.len);
    _memcpy(cat.str + a.len, b.str, b.len);

    return cat;
}
void StrLstPrint(StrLst *lst) {
    while (lst != NULL) {
        StrPrint(lst); // TODO: fix
        printf(", ");
        lst = lst->next;
    }
}
StrLst *StrSplit(MArena *arena, Str base, char split_at_and_remove) {
    // TODO: reimpl. using StrLstPut
    StrLst *first;
    StrLst *node;
    StrLst *prev = NULL;
    u32 i = 0;

    while (true) {
        while (base.str[i] == split_at_and_remove) {
            ++i;
        }
        if (i >= base.len) {
            return first;
        }

        node = (StrLst *) ArenaAlloc(arena, sizeof(StrLst));
        node->str = (char*) ArenaOpen(arena);
        node->len = 0;

        if (prev != NULL) {
            prev->next = node;
        }
        else {
            first = node;
        }

        int j = 0;
        while (base.str[i] != split_at_and_remove && i < base.len) {
            ++node->len;
            node->str[j] = base.str[i];
            ++i;
            ++j;
        }

        ArenaClose(arena, node->len);
        prev = node;
        if (i < base.len) {
            ++i; // skip the split char
        }
        else {
            return first;
        }
    }
}
// TODO: impl. "arena push version" e.g. a version that uses ArenaPush(src, len) rather than
//     ArenaAlloc() to see which one is most readable, I expect it to be better
// StringList *StrSplit(MArena *arena, String base, char split) {}

Str StrJoin(MArena *a, StrLst *strs) {
    Str join;
    join.str = (char*) ArenaOpen(a);
    join.len = 0;

    while (strs != NULL) {
        _memcpy(join.str + join.len, strs->str, strs->len);
        join.len += strs->len;
        strs = strs->next;
    }

    ArenaClose(a, join.len);
    return join;
}
Str StrJoinInsertChar(MArena *a, StrLst *strs, char insert) {
    Str join;
    join.str = (char*) ArenaOpen(a);
    join.len = 0;

    while (strs != NULL) {
        _memcpy(join.str + join.len, strs->str, strs->len);
        join.len += strs->len;
        strs = strs->next;

        if (strs != NULL) {
            join.str[join.len] = insert;
            ++join.len;
        }
    }

    ArenaClose(a, join.len);
    return join;
}


//
// string list builder functions, another take

StrLst *StrLstStart(MArena *a) {
    return (StrLst*) ArenaAlloc(a, 0);
}
StrLst *StrLstPut(MArena *a, char *str, StrLst *after = NULL) {
    StrLst _;
    StrLst *lst = (StrLst*) ArenaPush(a, &_, sizeof(StrLst)); // can we have an ArenaPush(type) for this ideom? T
    lst->len = _strlen(str);
    lst->str = (char*) ArenaAlloc(a, lst->len); // TODO: would be easy to put a T here to avoid the cast
    for (int i = 0; i < lst->len; ++i) {
        lst->str[i] = str[i];
    }
    if (after != NULL) {
        after->next = lst;
    }
    return lst;
}
void StrLstPrint(StrLst lst) {
    StrLst *iter = &lst;
    do {
        StrPrint(iter);
        printf("\n");
    }
    while ((iter = iter->next) != NULL);
}

// NOTE: "Hot" arena usage infers an assumption of pointer contguity.
//       E.g. our ptr, here "to", was the most recently allocated on a.
void StrCatHot(MArena *a, char *str, StrLst *to) {
    u8 *dest = (u8*) ArenaAlloc(a, _strlen(str));

    assert(to != NULL);
    assert(dest == (u8*) to->str + to->len);

    while (*str != '\0') {
        to->str[to->len++] = *str;
        ++str;
    }
}
void StrAppendHot(MArena *a, char c, StrLst *to) {
    u8 *dest = (u8*) ArenaAlloc(a, 1);

    assert(to != NULL);
    assert(dest == (u8*) to->str + to->len); // contiguity

    to->str[to->len++] = c;
}

#endif
