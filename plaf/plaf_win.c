#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fileapi.h>


// TODO: win impl. GetFilesInFolder, LoadFile, LoadFileMMAP, ReadSystemTimerMySec, ReadCPUTimer / QueryPerformanceCounter 
// TODO: linux impl. LoadFile
// TODO: put function headers in one place (platform.c?)
// TODO: test random numbers / gettimeofday or make a platform function that does something else on windows 


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
    // TODO: impl.
    return 0;
}
u64 ReadCPUTimer() {
    // TODO: impl.
    return 0;
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


//
// via S.O. 10905892:


// MSVC defines this in winsock2.h!?
//struct timeval {
//    long tv_sec;
//    long tv_usec;
//};
int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}