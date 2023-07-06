//#include "base.c"
//#include "memory.c"

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
