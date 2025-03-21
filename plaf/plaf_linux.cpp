#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdlib>


// TODO: experiment with <x86intrin.h> alongside <sys/time.h> for the straight up __rdtsc() call
// TODO: impl. LoadFile


//
// memory.c


u64 MemoryProtect(void *from, u64 amount) {
    mprotect(from, amount, PROT_READ | PROT_WRITE);
    return amount;
}
void *MemoryReserve(u64 amount) {
    void *result;

    result = (u8*) mmap(NULL, amount, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    return result;
}
s32 MemoryUnmap(void *at, u64 amount_reserved) {
    s32 ret = munmap(at, amount_reserved);
    return ret;
}



//
// profile.c


u64 ReadSystemTimerMySec() {
    u64 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

    return systime;
}
u32 ReadSystemTimerMySec32() {
    u32 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

    return systime;
}
u64 ReadCPUTimer() {
    u64 ticks = 0;
    #ifndef __arm__
    #ifndef __aarch64__
    ticks = __builtin_ia32_rdtsc(); // gcc
    #endif
    #endif
    // TODO: test __rdtsc(); with x86intrin.h ! (might increase compile time by a lot)
    return ticks;
}


//
// utils.c


void XSleep(u32 ms) {
    usleep(1000 * ms);
}

u8 *LoadFileMMAP(char *filepath, u64 *size_bytes) {
    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        printf("Could not open file: %s\n", filepath);
        return NULL;
    }

    s32 fd = fileno(f);
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        printf("Could not get file size: %s\n", filepath);
        return NULL;
    }

    u8 *data = (u8*) mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE | MAP_SHARED, fd, 0);
    if (size_bytes != NULL) {
        *size_bytes = sb.st_size;
    }
    fclose(f);
    return data;
}
StrLst *GetFilesInFolderPaths(MArena *unused, char *rootpath) {
    StrLst *first = NULL;

    struct dirent *dir;
    DIR *d = opendir(rootpath);
    if (d) {
        d = opendir(rootpath);

        Str path = StrLiteral(rootpath);
        if (path.len == 1 && path.str[0] == '.') {
            path.len = 0;
        }
        else if (path.str[path.len-1] != '/') {
            path = StrCat(path, "/");
        }

        StrLst *lst = NULL;
        while ((dir = readdir(d)) != NULL) {
            // omit "." and ".."
            if (!_strcmp(dir->d_name, ".") || !_strcmp(dir->d_name, "..")) {
                continue;
            }

            Str dname = StrCat( path, StrLiteral(dir->d_name) );
            lst = StrLstPut(dname, lst);
            if (first == NULL) {
                first = lst;
            }
            //Str StrCat( lst->GetStr(), StrLiteral(dir->d_name), );
        }
        closedir(d);
    }
    return first;
}
bool SaveFile(char *filepath, u8 *data, u32 len) {
    FILE *f = fopen(filepath, "w");

    if (f == NULL) {
        printf("SaveFile: Could not open file %s\n", filepath);
        return false;
    }

    fwrite(data, 1, len, f);
    fclose(f);

    return true;
}
