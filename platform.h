#ifndef __PLATFORM_H__
#define __PLATFORM_H__


// platform-specific implementations of functions
// not currently distributed in separate files


const char *getBuild() { // courtesy of S.O.
    #if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
    #elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    return "x86_32";
    #elif defined(__ARM_ARCH_2__)
    return "ARM2";
    #elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
    return "ARM3";
    #elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
    return "ARM4T";
    #elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
    return "ARM5"
    #elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
    return "ARM6T2";
    #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
    return "ARM6";
    #elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    return "ARM7";
    #elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    return "ARM7A";
    #elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    return "ARM7R";
    #elif defined(__ARM_ARCH_7M__)
    return "ARM7M";
    #elif defined(__ARM_ARCH_7S__)
    return "ARM7S";
    #elif defined(__aarch64__) || defined(_M_ARM64)
    return "ARM64";
    #elif defined(mips) || defined(__mips__) || defined(__mips)
    return "MIPS";
    #elif defined(__sh__)
    return "SUPERH";
    #elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
    return "POWERPC";
    #elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
    return "POWERPC64";
    #elif defined(__sparc__) || defined(__sparc)
    return "SPARC";
    #elif defined(__m68k__)
    return "M68K";
    #else
    return "UNKNOWN";
    #endif
}


#if defined __linux__ || defined __linux
    #define LINUX 1
    #define WINDOWS 0

    #ifdef __arm__
        #define RPI 1
    #endif
    #ifdef __aarch64__
        #define RPI 1
    #endif

    #ifndef RPI
    #define RPI 0
    #endif


    //
    //  plaf_lnx.cpp


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
        //  memory.h

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
                    lst = StrLstPush(dname, lst);
                    if (first == NULL) {
                        first = lst;
                    }
                    //Str StrCat( lst->GetStr(), StrLiteral(dir->d_name), );
                }
                closedir(d);
            }
            return first;
        }
        StrLst *GetFilesInFolderPaths_Rec(char *rootpath, StrLst *first = NULL, StrLst *last = NULL, const char *extension_filter = NULL, bool do_recurse = true) {

            struct dirent *dir_entry;
            if (DIR *dir = opendir(rootpath)) {
                dir = opendir(rootpath);

                Str path = StrLiteral(rootpath);
                if (path.len == 1 && path.str[0] == '.') {
                    path.len = 0;
                }
                else if (path.str[path.len-1] != '/') {
                    path = StrCat(path, "/");
                }

                while ((dir_entry = readdir(dir)) != NULL) {
                    if (!_strcmp(dir_entry->d_name, ".") || !_strcmp(dir_entry->d_name, "..")) {
                        continue;
                    }

                    Str file_path = StrCat( path, StrLiteral(dir_entry->d_name) );
                    if (first == NULL) {
                        first = last;
                    }

                    if (dir_entry->d_type == 4) { // recurse into directory
                        last = GetFilesInFolderPaths_Rec( StrZeroTerm(file_path), first, last, extension_filter, do_recurse);
                    }
                    else {
                        if (extension_filter == NULL || StrEqual(StrExtension(StrL(dir_entry->d_name)), extension_filter)) {
                            last = StrLstPush(file_path, last);
                            if (false) { StrPrint("", file_path, "\n"); }
                        }
                    }
                }
                closedir(dir);
            }

            else if (FILE *f = fopen(rootpath, "r")) {
                last = StrLstPush(rootpath, last);
            }

            return last;
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

#else 
    #define LINUX 0
    #define WINDOWS 1
    #define RPI 0


    //
    //  plaf_win.cpp


        #define WIN32_LEAN_AND_MEAN
        #define NOMINMAX
        #include <windows.h>
        #include <fstream>
        #include <fileapi.h>
        #include <intrin.h>

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
        s32 MemoryUnmap(void *at, u64 amount_reserved) {
            bool ans = VirtualFree(at, 0, MEM_RELEASE);
            if (ans == true) {
                return 0;
            }
            else {
                return 1;
            }
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
        StrLst *GetFilesInFolderPaths_Rec(char *rootpath, StrLst *first = NULL, StrLst *last = NULL, const char *extension_filter = NULL, bool do_recurse = true) {
            // TODO: impl.
        }
        bool SaveFile(char *filepath, u8 *data, u32 len) {
            std::ofstream outstream(filepath, std::ios::out | std::ios::binary);
            outstream.write((const char*) data, len);
            bool result = outstream.good();
            outstream.close();

            return result;
        }

#endif


#if LINUX
#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif
#ifndef DLL_IMPORT
#define DLL_IMPORT
#endif
#elif WINDOWS
#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif
#ifndef DLL_IMPORT
#define DLL_IMPORT __declspec(dllimport)
#endif
#endif


#ifndef DLL_CLIENT
#ifndef DLL_IMPORTEXPORT
#define DLL_IMPORTEXPORT DLL_EXPORT
#endif
#else
#ifndef DLL_IMPORTEXPORT
#define DLL_IMPORTEXPORT DLL_IMPORT
#endif
#endif


#endif
