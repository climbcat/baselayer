#include <cstdio>
#include <cassert>


// NOTE: currently no "string list header" struct, which means that strings and str lst are
//       treated a bit differently: Strings are passed as a struct, but str lists as a pointer.
// NOTE: the string list is in fact an LList1, maybe use the functions from base.c


struct String {
    char *str = NULL;
    u32 len = 0;
};

struct StringList {
    StringList *next = NULL;
    String value;
};

String StrLiteral(MArena *a, const char *lit) {
    String s;
    s.len = 0;
    while (*(lit + s.len) != '\0') {
        ++s.len;
    }
    s.str = (char*) ArenaAlloc(a, s.len);
    memcpy(s.str, lit, s.len);

    return s;
}
void StrPrint(const char *format, String s) {
    u8 format_str_max_len = 255;
    char buff[s.len + format_str_max_len];
    sprintf(buff, "%.*s", s.len, s.str);
    printf(format, buff);
}
void StrPrint(String s) {
    printf("%.*s", s.len, s.str);
}
bool StrEqual(String a, String b) {
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
String StrCat(MArena *arena, String a, String b) {
    String cat;
    cat.len = a.len + b.len;
    cat.str = (char*) ArenaAlloc(arena, cat.len);
    memcpy(cat.str, a.str, a.len);
    memcpy(cat.str + a.len, b.str, b.len);

    return cat;
}
void StrLstPrint(StringList *lst) {
    while (lst != NULL) {
        StrPrint(lst->value);
        printf(", ");
        lst = lst->next;
    }
}
StringList *StrSplit(MArena *arena, String base, char split_at_and_remove) {
    StringList *first;
    StringList *node;
    StringList *prev = NULL;
    u32 i = 0;

    while (true) {
        while (base.str[i] == split_at_and_remove) {
            ++i;
        }
        if (i >= base.len) {
            return first;
        }

        node = (StringList *) ArenaAlloc(arena, sizeof(StringList));
        node->value.str = (char*) ArenaOpen(arena);
        node->value.len = 0;

        if (prev != NULL) {
            prev->next = node;
        }
        else {
            first = node;
        }

        int j = 0;
        while (base.str[i] != split_at_and_remove && i < base.len) {
            ++node->value.len;
            node->value.str[j] = base.str[i];
            ++i;
            ++j;
        }

        ArenaClose(arena, node->value.len);
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

String StrJoin(MArena *a, StringList *strs) {
    String join;
    join.str = (char*) ArenaOpen(a);
    join.len = 0;

    while (strs != NULL) {
        memcpy(join.str + join.len, strs->value.str, strs->value.len);
        join.len += strs->value.len;
        strs = strs->next;
    }

    ArenaClose(a, join.len);
    return join;
}
String StrJoinInsertChar(MArena *a, StringList *strs, char insert) {
    String join;
    join.str = (char*) ArenaOpen(a);
    join.len = 0;

    while (strs != NULL) {
        memcpy(join.str + join.len, strs->value.str, strs->value.len);
        join.len += strs->value.len;
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

struct Str {
    char *str = NULL;
    u32 len = 0;
};

struct StrLst {
    StrLst *next = NULL;
    char *str = NULL;
    u32 len = 0;
};

StrLst *StrLstStart(MArena *a) {
    return (StrLst*) ArenaAlloc(a, 0);
}
StrLst *StrLstPut(MArena *a, char *str, StrLst *after = NULL) {
    StrLst _;
    StrLst *lst = (StrLst*) ArenaPush(a, &_, sizeof(StrLst)); // can we have an ArenaPush(type) for this ideom? T
    lst->len = strlen(str);
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
    // print lines of strs
    StrLst *iter = &lst;
    do {
        u8 format_str_max_len = 255;
        char buff[iter->len + format_str_max_len];
        sprintf(buff, "%.*s", iter->len, iter->str);
        printf("%s\n", buff);
    }
    while ((iter = iter->next) != NULL);
}

// NOTE: "Hot" arena usage infers an assumption of pointer contguity.
//       E.g. our ptr, here "to", was the most recently allocated on a.
void StrCatHot(MArena *a, char *str, StrLst *to) {
    u8 *dest = (u8*) ArenaAlloc(a, strlen(str));

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
