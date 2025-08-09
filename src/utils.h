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

StrLst *GetFiles(char *rootpath, const char *extension_filter = NULL, bool do_recurse = true);

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

    Str result = {};
    if (size && data) {
        result = { (char*) data, size };
    }
    return result;
}

void *LoadFileFSeek(MArena *a_dest, const char *filepath, u32 *size = NULL) {
    return LoadFileFSeek(a_dest, (char*) filepath, size);
}

Str LoadTextFile(MArena *a_files, const char *f_path) {
    Str f_str = {};
    f_str.str = (char*) LoadFileFSeek(a_files, f_path, &f_str.len);

    return f_str;
}

Str LoadTextFile(MArena *a_files, Str f_path) {
    Str f_str = {};
    f_str.str = (char*) LoadFileFSeek(a_files, StrZ(f_path), &f_str.len);

    return f_str;
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


Str GetYYMMDD() {
    Str s = {};

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
