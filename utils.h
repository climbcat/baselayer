#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdlib>
#include <cassert>


#include "string.h"


//
// Other

void XSleep(u32 ms);



//
// file I/O


u8 *LoadFileMMAP(char *filepath, u64 *size_bytes);
u8 *LoadFileMMAP(const char *filepath, u64 *size_bytes) {
    return LoadFileMMAP((char*) filepath, size_bytes);
}
StrLst *GetFilesInFolderPaths(MArena *a, char *rootpath);
StrLst *GetFilesInFolderPaths(MArena *a, const char *rootpath) {
    return GetFilesInFolderPaths(a, (char*) rootpath);
}

u32 LoadFileFSeek(char* filepath, u8* dest) {
    assert(dest != NULL && "data destination must be valid");
    u32 len = 0;

    FILE * f = fopen(filepath, "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        fseek(f, 0, SEEK_SET);
        fread(dest, 1, len, f);
        fclose(f);
    }
    else {
        printf("LoadFileFSeek: Could not open file: %s\n", filepath);
    }

    return len;
}

bool SaveFile(char *filepath, u8 *data, u32 len);
bool SaveFile(const char *filepath, u8 *data, u32 len) {
    return SaveFile((char *)filepath, data, len);
}
bool ArenaSave(MArena *a, char *filename) {
    return SaveFile(filename, a->mem, (u32) a->used);
}
bool ArenaSave(MArena *a, const char *filename) {
    return ArenaSave(a, (char *) filename);
}


//
// path / filename stuff


Str StrBasename(char *path) {
    assert(g_a_strings != NULL && "init strings first");

    Str before_ext = StrSplit(StrLiteral(path), '.')->GetStr();
    StrLst* slashes = StrSplit(before_ext, '/');
    while (slashes->next) {
        slashes = slashes->next;
    }
    return slashes->GetStr();
}
Str StrBasename(Str path) {
    return StrBasename(StrZeroTerm(path));
}
Str StrExtension(char *path) {
    assert(g_a_strings != NULL && "init strings first");

    Str s { NULL, 0 };
    StrLst *lst = StrSplit(StrLiteral(path), '.');
    if (lst->next != NULL) {
        s = lst->next->GetStr();
    }
    return s;
}
Str StrExtension(Str path) {
    return StrExtension(StrZeroTerm(path));
}
StrLst *GetFilesExt(const char *extension, const char *path = ".") {
    StrLst *all = GetFilesInFolderPaths(InitStrings(), path);
    StrLst *filtered = NULL;
    StrLst *first = NULL;
    Str ext = StrLiteral(extension);
    while (all != NULL) {
        Str _fpath = all->GetStr();
        Str _ext = StrExtension(_fpath);

        if (StrEqual(_ext, ext)) {
            filtered = StrLstPut(_fpath, filtered);
            if (first == NULL) {
                first = filtered;
            }
        }
        all = all->next;
    }
    return first;
}


//
// Baselayer initialization


MContext *InitBaselayer() {
    RandInit();
    StringCreateArena();
    return GetContext();
}


#endif
