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


char *LoadFileMMAP(char *filepath, u64 *size_bytes) {
    // TODO: finish impl.
    /*
    char *szName = "somefile.bin";
    char *szMsg = "Message from first process.";
    HANDLE mapfileobj = CreateFileMapping(
                INVALID_HANDLE_VALUE,   // use paging file
                NULL,                   // default security
                PAGE_READWRITE,         // read/write access
                0,                      // maximum object size (high-order DWORD)
                256,                    // maximum object size (low-order DWORD)
                szName);                // name of mapping object
    u8* data = (u8*) MapViewOfFile(
                mapfileobj,             // handle to map object
                FILE_MAP_ALL_ACCESS,    // read/write permission
                0,
                0,
                BUF_SIZE);
    */
    if (size_bytes != NULL) {
        *size_bytes = 0;
    }
    return NULL;
}
char *LoadFile(char *filepath, u64 *size_bytes) {
    /*
    char *szName = "somefile.bin";
    char *szMsg = "Message from first process.";
    HANDLE hFile = CreateFile(filepath,               // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                       NULL);                 // no attr. template
    u32 filesz = GetFileSize(hFile,  NULL);

    printf("%d\n", filesz);
    ReadFileEx(hFile, ReadBuffer, BUFFERSIZE-1, &ol, FileIOCompletionRoutine);
    */
    if (size_bytes != NULL) {
        *size_bytes = 0;
    }
    return NULL;
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



