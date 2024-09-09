#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <fstream>
#include <fileapi.h>
#include <intrin.h>


// TODO: win impl. GetFilesInFolderPaths, LoadFile, LoadFileMMAP


//
// memory.h


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
// profile.h


u64 ReadSystemTimerMySec() {
    // systime (via S.O. 10905892)

    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;
    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    long tv_sec  = (long) ((time - EPOCH) / 10000000L);
    long tv_usec = (long) (system_time.wMilliseconds * 1000);
    u64 systime = (u32) tv_sec*1000000 + tv_usec; // microsecs 
    return systime;
}
u32 ReadSystemTimerMySec32() {
    // systime (via S.O. 10905892)

    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;
    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    long tv_sec  = (long) ((time - EPOCH) / 10000000L);
    long tv_usec = (long) (system_time.wMilliseconds * 1000);
    u32 systime = (u32) tv_sec*1000000 + tv_usec; // microsecs 

    return systime;
}
u64 ReadCPUTimer() {
    u64 rd = __rdtsc();
    return rd;
}


//
// utils.h


void XSleep(u32 ms) {
    Sleep(ms);
}


u8 *LoadFileMMAP(char *filepath, u64 *size_bytes) {
    HANDLE f, map;
    LPVOID lpBasePtr;
    LARGE_INTEGER fsz;

    f = CreateFile(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (f == INVALID_HANDLE_VALUE) {
        printf("Could not open file: %s\n", filepath);
        return NULL;
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
        return NULL;
    }

    return data;
}
StrLst *GetFilesInFolderPaths(MArena *a, char *rootpath) {
    StrLst *first = NULL;


    WIN32_FIND_DATA fd_file;
    HANDLE h_find = FindFirstFile(rootpath, &fd_file);
    if (h_find == INVALID_HANDLE_VALUE) {
        printf("invalid: %s\n", rootpath);
    }

    char with_windows_star[200];
    sprintf(with_windows_star, "%s/*", rootpath);
    h_find = FindFirstFile(with_windows_star, &fd_file);
    if (h_find != NULL) {
        StrLst *lst = NULL;

        Str path = StrLiteral(rootpath);
        if (path.len == 1 && path.str[0] == '.') {
            path.len = 0;
        }
        else if (path.str[path.len-1] != '/') {
            path = StrCat(path, "/");
        }

        while (FindNextFile(h_find, &fd_file)) {
            // omit "." and ".."
            if (!_strcmp(fd_file.cFileName, ".") || !_strcmp(fd_file.cFileName, "..")) {
                continue;
            }

            // next strlst node
            lst = StrLstPut(a, rootpath, lst);

            Str dname = StrCat( path, StrLiteral(fd_file.cFileName) );
            lst = StrLstPut(dname, lst);
            if (first == NULL) {
                first = lst;
            }
        }
        FindClose(h_find);
    }

    return first;
}
bool SaveFile(char *filepath, u8 *data, u32 len) {
    std::ofstream outstream(filepath, std::ios::out | std::ios::binary);
    outstream.write((const char*) data, len);
    bool result = outstream.good();
    outstream.close();

    return result;
}
