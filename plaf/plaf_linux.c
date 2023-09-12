#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <dirent.h>

#include <cstdlib>

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

//
// profile.c

u64 ReadSystemTimerMySec() {
    u64 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

    return systime;
}
u64 ReadCPUTimer() {
    u64 ticks = __builtin_ia32_rdtsc(); // gcc
    return ticks;
}

//
// utils.c

char *LoadFileMMAP(char *filepath, u64 *size_bytes) {
    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        printf("Could not open file: %s\n", filepath);
        exit(1);
    }

    s32 fd = fileno(f);
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        printf("Could not get file size: %s\n", filepath);
        exit(1);
    }

    char *str = (char*) mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE | MAP_SHARED, fd, 0);
    if (size_bytes != NULL) {
        *size_bytes = sb.st_size;
    }

    fclose(f);
    return str;
}
StrLst GetFilesInFolderPaths(MArena *a, char *rootpath) {
    u32 rootpath_len = strlen(rootpath);
    bool needslash = rootpath[rootpath_len-1] != '/';
    StrLst *lst = NULL;
    StrLst *first = (StrLst*) ArenaAlloc(a, 0);
    
    struct dirent *dir;
    DIR *d = opendir(rootpath);
    if (d) {
        d = opendir(rootpath);
        while ((dir = readdir(d)) != NULL) {

            // next strlst node
            lst = StrLstPut(a, rootpath, lst);

            // hot catenation
            if (needslash) {
                StrAppendHot(a, '/', lst);
            }
            StrCatHot(a, dir->d_name, lst);
        }
        closedir(d);
    }

    return *first;
}