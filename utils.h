#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdlib>
#include <cassert>
#include <time.h>


#include "string.h"


//
//  Other


void XSleep(u32 ms);


//
//  File I/O


u8 *LoadFileMMAP(char *filepath, u64 *size_bytes);

u8 *LoadFileMMAP(const char *filepath, u64 *size_bytes) {
    return LoadFileMMAP((char*) filepath, size_bytes);
}

StrLst *GetFilesInFolderPaths(MArena *a, char *rootpath);

StrLst *GetFilesInFolderPaths_Rec(char *rootpath, StrLst *first = NULL, StrLst *last = NULL, const char *extension_filter = NULL, bool do_recurse = true);

StrLst *GetFilesInFolderPaths(MArena *a, const char *rootpath) {
    return GetFilesInFolderPaths(a, (char*) rootpath);
}

u32 LoadFileGetSize(char* filepath) {
    FILE * f = fopen(filepath, "r");
    if (f == NULL) {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    u32 len = ftell(f);
    return len;
}

u32 LoadFileGetSize(const char* filepath) {
    return LoadFileGetSize((char*) filepath);
}

u32 LoadFileFSeek(char* filepath, void* dest) {
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

void *LoadFileFSeek(MArena *a_dest, char *filepath, u32 *size = NULL) {
    u32 sz = LoadFileGetSize(filepath);
    if (sz == 0) {
        return NULL;
    }

    void *dest = ArenaAlloc(a_dest, sz);
    LoadFileFSeek(filepath, dest);

    if (size != NULL) {
        *size = sz;
    }
    return dest;
}

void *LoadFileFSeek(MArena *a_dest, Str filepath, u32 *size = NULL) {
    assert(filepath.len < 1024);
    char filepath_zt[1024];
    sprintf(filepath_zt, "%.*s", filepath.len, filepath.str);
    filepath_zt[filepath.len] = 0;

    void *data = LoadFileFSeek(a_dest, filepath_zt, size);
    return data;
}

Str LoadTextFileFSeek(MArena *a_dest, Str filepath) {
    char filepath_zt[1024];
    sprintf(filepath_zt, "%.*s", filepath.len, filepath.str);
    filepath_zt[filepath.len] = 0;

    u32 size;
    void *data = LoadFileFSeek(a_dest, filepath_zt, &size);
    return Str { (char*) data, size };
}

void *LoadFileFSeek(MArena *a_dest, const char *filepath, u32 *size = NULL) {
    return LoadFileFSeek(a_dest, (char*) filepath, size);
}

bool SaveFile(char *filepath, u8 *data, u32 len);

bool SaveFile(const char *filepath, u8 *data, u32 len) {
    // const char star
    return SaveFile((char *)filepath, data, len);
}

bool SaveFile(char *filepath, void *data, u32 len) {
    // void star
    return SaveFile((char *)filepath, (u8*)data, len);
}

bool SaveFile(const char *filepath, void *data, u32 len) {
    // const char star and void star
    return SaveFile((char *)filepath, (u8*)data, len);
}

bool ArenaSave(MArena *a, char *filename) {
    return SaveFile(filename, a->mem, (u32) a->used);
}

bool ArenaSave(MArena *a, const char *filename) {
    return ArenaSave(a, (char *) filename);
}


//
//  Path / filename helpers


Str StrBasename(char *path) {
    assert(g_a_strings != NULL && "init strings first");

    // TODO: shouldn't this fail?

    Str before_ext = StrSplit(StrL(path), '.')->GetStr();
    StrLst* slashes = StrSplit(before_ext, '/');
    while (slashes->next) {
        slashes = slashes->next;
    }
    return slashes->GetStr();
}

Str StrBasename(MArena *a, char *path) {
    assert(g_a_strings != NULL && "init strings first");

    // TODO: shouldn't this fail?

    Str before_ext = StrSplit(StrL(path), '.')->GetStr();
    StrLst* slashes = StrSplit(before_ext, '/');
    while (slashes->next) {
        slashes = slashes->next;
    }
    return slashes->GetStr();
}

Str StrBasename(Str path) {
    return StrBasename(StrZeroTerm(path));
}

Str StrExtension(MArena *a, char *path) {
    Str s { NULL, 0 };
    StrLst *lst = StrSplit(StrL(path), '.');
    if (lst->next != NULL) {
        s = lst->next->GetStr();
    }
    return s;
}

Str StrExtension(char *path) {
    assert(g_a_strings != NULL && "init strings first");

    Str s { NULL, 0 };
    StrLst *lst = StrSplit(StrL(path), '.');
    while (lst->next != NULL) {
        lst = lst->next;
    }
    s = lst->GetStr();
    return s;
}

Str StrExtension(Str path) {
    return StrExtension(StrZeroTerm(path));
}

Str StrDirPath(Str path) {
    assert(g_a_strings != NULL && "init strings first");

    StrLst *slash = StrSplit(path, '/');
    u32 len = StrListLen(slash);

    Str cat = StrL("");
    for (u32 i = 0; i < len - 1; ++i) {
        cat = StrCat(cat, slash->GetStr());
        cat = StrCat(cat, StrL("/"));
        slash = slash->next;
    }
    return cat;
}

Str StrPathBuild(Str dirname, Str basename, Str ext) {
    dirname = StrTrim(dirname, '/');
    basename = StrTrim(basename, '/');

    Str path = dirname;
    if (dirname.len) {
       path = StrCat(dirname, StrL("/"));
    }
    path = StrCat(path, basename);
    path = StrCat(path, StrL("."));
    path = StrCat(path, ext);

    return path;
}

Str StrPathJoin(Str path_1, Str path_2) {
    Str path = StrCat(path_1, StrL("/"));
    path = StrCat(path, path_2);
    return path;
}

StrLst *GetFilesExt(const char *extension, const char *path = ".") {
    StrLst *all = GetFilesInFolderPaths(InitStrings(), path);
    StrLst *filtered = NULL;
    StrLst *first = NULL;
    Str ext = StrL(extension);
    while (all != NULL) {
        Str _fpath = all->GetStr();
        Str _ext = StrExtension(_fpath);

        if (StrEqual(_ext, ext)) {
            filtered = StrLstPush(_fpath, filtered);
            if (first == NULL) {
                first = filtered;
            }
        }
        all = all->next;
    }
    return first;
}


//
//  FInfo: file path info, construct file names / paths


struct FInfo {
    Str name;
    Str ext;
    Str basename;
    Str path;
    Str dirname;

    Str BuildName(const char *prefix, const char *suffix, const char *ext) {
        Str bn_new = StrL(prefix);
        bn_new = StrCat(bn_new, basename);
        bn_new = StrCat(bn_new, StrL(suffix));
        Str rebuilt = StrPathBuild(dirname, bn_new, StrL(ext));
        return rebuilt;
    }
    char *BuildNameZ(const char *prefix, const char *suffix, const char *ext) {
        return StrZeroTerm( this->BuildName(prefix, suffix, ext) );
    }
    Str StripDirname() {
        Str filename = StrL("");
        filename = StrCat(filename, basename);
        filename = StrCat(filename, StrL("."));
        filename = StrCat(filename, ext);
        return filename;
    }
    void Print() {
        StrPrint("name      : ", name, "\n");
        StrPrint("extension : ", ext, "\n");
        StrPrint("basename  : ", basename, "\n");
        StrPrint("dirname   : ", dirname, "\n");
        Str rebuilt = StrPathBuild(dirname, basename, ext);
        StrPrint("rebuilt   : ", rebuilt, "\n");
    }
};

FInfo FInfoGet(Str pathname) {
    FInfo info;
    info.name = pathname;
    info.ext = StrExtension(pathname);
    info.basename = StrBasename(pathname);
    info.dirname = StrDirPath(pathname);
    return info;
}

FInfo InitFInfo(Str pathname) {
    return FInfoGet(pathname);
}

inline
FInfo FInfoGet(const char*pathname) {
    return FInfoGet(StrL(pathname));
}


Str GetYYMMDD() {
    Str s;

    time_t t = time(NULL);
    struct tm loc = *localtime(&t);
    char buff[34];
    sprintf(buff, "%02d%02d%02d",
        loc.tm_year - 100,
        loc.tm_mon + 1,
        loc.tm_mday);
    printf("%s\n", buff);

    return s;
}


#endif
