#include <cstdlib>
#include <cassert>
//#include <sys/stat.h>
//#include <sys/time.h>

// Linux:
//#include <sys/mman.h>
//#include <dirent.h>
// Windows:
#include <windows.h>



//
// file I/O


// Linux:
/*
char *LoadFileMMAP(char *filepath, u64 *size_bytes = NULL) {
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
*/
/*
char *szName = "somefile.bin";
char *szMsg = "Message from first process.";

char *LoadFileMMAP(char *filepath, u64 *size_bytes = NULL) {
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

}
*/

// Windows:
/*
#include <fileapi.h>
char *LoadFile(char *filepath, u64 *size_bytes = NULL) {
    HANDLE hFile = CreateFile(filepath,               // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                       NULL);                 // no attr. template
    u32 filesz = GetFileSize(hFile,  NULL);

    printf("%d\n", filesz);
    //ReadFileEx(hFile, ReadBuffer, BUFFERSIZE-1, &ol, FileIOCompletionRoutine);
}
*/




// Linux:
/*
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
*/


//
// Windows: via S.O. 10905892:

//#include <stdint.h> // portable: uint64_t   MSVC: __int64 
// MSVC defines this in winsock2.h!?
struct timeval {
    long tv_sec;
    long tv_usec;
};
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


//
// random


#ifndef ULONG_MAX
#  define ULONG_MAX ((unsigned long)0xffffffffffffffffUL)
#endif


void Kiss_SRandom(unsigned long state[7], unsigned long seed) {
    if (seed == 0) seed = 1;
    state[0] = seed | 1; // x
    state[1] = seed | 2; // y
    state[2] = seed | 4; // z
    state[3] = seed | 8; // w
    state[4] = 0;        // carry
}
unsigned long Kiss_Random(unsigned long state[7]) {
    state[0] = state[0] * 69069 + 1;
    state[1] ^= state[1] << 13;
    state[1] ^= state[1] >> 17;
    state[1] ^= state[1] << 5;
    state[5] = (state[2] >> 2) + (state[3] >> 3) + (state[4] >> 2);
    state[6] = state[3] + state[3] + state[2] + state[4];
    state[2] = state[3];
    state[3] = state[6];
    state[4] = state[5] >> 30;
    return state[0] + state[1] + state[3];
}
unsigned long _hash(unsigned long x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
unsigned long g_state[7];
bool g_didinit = false;
#define Random() Kiss_Random(g_state)
u32 RandInit(u32 seed = 0) {
    if (g_didinit == true)
        return 0;

    if (seed == 0) {
        timeval tm;
        gettimeofday(&tm, NULL);
        seed = _hash((unsigned long) tm.tv_sec*1000000 + tm.tv_usec);
    }
    Kiss_SRandom(g_state, seed);

    g_didinit = true;
    return seed;
}

double Rand01() {
    double randnum;
    randnum = (double) Random();
    randnum /= (double) ULONG_MAX + 1;
    return randnum;
}
double RandPM1() {
    double randnum;
    randnum = (double) Random();
    randnum /= ((double) ULONG_MAX + 1) / 2;
    randnum -= 1;
    return randnum;
}

int RandMinMaxI(int min, int max) {
    assert(max > min);
    return Random() % (max - min + 1) + min;
}
