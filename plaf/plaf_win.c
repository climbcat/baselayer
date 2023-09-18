#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fileapi.h>
#include <intrin.h>


// TODO: win impl. GetFilesInFolderPaths, LoadFile, LoadFileMMAP


//
// memory.c


u64 MemoryProtect(void *from, u64 amount) {
    VirtualAlloc(from, amount, MEM_COMMIT, PAGE_READWRITE);
    return amount;
}
void *MemoryReserve(u64 amount) {
    void *result;
    result = VirtualAlloc(NULL, amount, MEM_RESERVE, PAGE_NOACCESS);
    return result;
}


//
// profile.c


u64 ReadSystemTimerMySec() {
	LARGE_INTEGER val;
	QueryPerformanceCounter(&val);
	return val.QuadPart;
}
u64 ReadCPUTimer() {
    u64 rd = __rdtsc();
    return rd;
}


//
// utils.c


u8 *LoadFileMMAP(char *filepath, u64 *size_bytes) {
    HANDLE f, map;
    LPVOID lpBasePtr;
    LARGE_INTEGER fsz;

    f = CreateFile(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (f == INVALID_HANDLE_VALUE) {
        printf("Could not open file: %s\n", filepath);
        exit(1);
    }

    if (!GetFileSizeEx(f, &fsz)) {
        fprintf(stderr, "GetFileSize failed with error %d\n", GetLastError());
        CloseHandle(f);
        return NULL;
    }
    if (size_bytes != NULL) {
        *size_bytes = fsz.QuadPart;
    }

    map = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
    u8 *data = (u8*) MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
    if (data == NULL) {
        printf("Could not load file: %s\n", filepath);
        exit(1);
    }

    return data;
}
StrLst GetFilesInFolderPaths(MArena *a, char *rootpath) {
    WIN32_FIND_DATA fd_file;

    HANDLE h_find = FindFirstFile(rootpath, &fd_file);

    if (h_find == INVALID_HANDLE_VALUE) {
        printf("invalid\n");
    }

    u32 rootpath_len = _strlen(rootpath);
    bool needslash = rootpath[rootpath_len-1] != '/';
    StrLst *lst = NULL;
    StrLst *first = (StrLst*) ArenaAlloc(a, 0);

    char with_windows_star[200];
    sprintf(with_windows_star, "%s/*", rootpath);
    h_find = FindFirstFile(with_windows_star, &fd_file);
    if (h_find != NULL) {
        do {
            // omit "." and ".."
            if (!_strcmp(fd_file.cFileName, ".") || !_strcmp(fd_file.cFileName, "..")) {
                continue;
            }

            // next strlst node
            lst = StrLstPut(a, rootpath, lst);

            // hot catenation
            if (needslash) {
                StrAppendHot(a, '/', lst);
            }
            StrCatHot(a, fd_file.cFileName, lst);
        }
        while (FindNextFile(h_find, &fd_file));
        FindClose(h_find);
    }

    return *first;
}
