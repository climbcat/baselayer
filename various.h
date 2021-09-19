#ifndef __VARIOUS_H__
#define __VARIOUS_H__

#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include <stdio.h>

#include "random.h"
#include "memory.h"


/**
* NOTE: Does not count the null char in strlen (just like the C function).
*/

void WriteRandomHexStr(char* dest, int strlen, bool put_nullchar = true, bool do_randinit = false) {
    if (do_randinit)
        RandInit();

    for (int i = 0; i < strlen ; i++) {
        switch (RandMinMaxI(0, 15)) {
            case 0: { *dest = '0'; break; }
            case 1: { *dest = '1'; break; }
            case 2: { *dest = '2'; break; }
            case 3: { *dest = '3'; break; }
            case 4: { *dest = '4'; break; }
            case 5: { *dest = '5'; break; }
            case 6: { *dest = '6'; break; }
            case 7: { *dest = '7'; break; }
            case 8: { *dest = '8'; break; }
            case 9: { *dest = '9'; break; }
            case 10: { *dest = 'a'; break; }
            case 11: { *dest = 'b'; break; }
            case 12: { *dest = 'c'; break; }
            case 13: { *dest = 'd'; break; }
            case 14: { *dest = 'e'; break; }
            case 15: { *dest = 'f'; break; }
            default: { assert(1==0); break; }
        };
        dest++;
    }

    if (put_nullchar) {
        *dest = '\0';
    }
}



char* LoadFilePath(char* filepath, StackAllocator *stack = NULL) {
    char * buffer = NULL;
    long length;
    FILE * f = fopen(filepath, "rb");

    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        if (stack != NULL) {
          buffer = (char*) stack->Alloc(length);
        }
        else {
          buffer = (char*) malloc(length);
        }
        if (buffer)
          fread (buffer, 1, length, f);
        fclose (f);
    }

    return buffer;
}


// TODO: use a (char *dest) argument rather than the allocator
char* LoadFile(char* filename, bool use_cwd = true, StackAllocator *stack = NULL) {
    if (use_cwd) {
        char cwd[KILOBYTE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            strcat(cwd, "/");
            strcat(cwd, filename);
            filename = (char*) cwd;

            return LoadFilePath(filename, stack);
        }
        return NULL;
    }
    else
        return LoadFilePath(filename, stack);
}

bool SaveToFile(char *filename, char *text) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return false;
    }

    fprintf(file, "%s", text);
    fclose(file);

    return true;
}

ArrayListT<char*> GetFilesInFolderPaths(char *rootpath, StackAllocator *stack) {
    ArrayListT<char*> result;
    struct dirent *dir;
    u32 count = 0;
    u32 rootpath_len = strlen(rootpath);

    DIR *d = opendir(rootpath);
    if (d) {

        while ((dir = readdir(d)) != NULL) {
            ++count;
        }
        closedir(d);
    }

    d = opendir(rootpath);
    if (d) {
        result.Init(stack->Alloc(sizeof(char*) * count));

        d = opendir(rootpath);
        while ((dir = readdir(d)) != NULL) {
            bool needslash = rootpath[rootpath_len-1] != '/';

            u32 len = rootpath_len + int(needslash) + strlen(dir->d_name) + 1;
            char* path = (char*) stack->Alloc( len );
            
            strcpy(path, rootpath);
            if (needslash) {
              strcat(path, "/");
            }
            strcat(path, dir->d_name);

            result.Add(&path);
        }
        closedir(d);
    }

    return result;
}

inline
float MinF(float a, float b) {
    if (a <= b)
        return a;
    else
        return b;
}
inline
float MaxF(float a, float b) {
    if (a >= b)
        return a;
    else
        return b;
}
inline
int MinI(int a, int b) {
    if (a <= b)
        return a;
    else
        return b;
}
inline
int MaxI(int a, int b) {
    if (a >= b)
       return a;
    else
        return b;
}

void Sleep(u32 time_ms) {
    usleep(time_ms * 1000);
}


#endif