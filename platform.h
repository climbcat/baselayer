#ifndef __PLATFORM_H__
#define __PLATFORM_H__

// platform-specific implementations of functions
// not currently distributed in separate files

#if defined __linux__ || defined __linux
    #define LINUX 1
    #define WINDOWS 0
    #include "plaf/plaf_linux.c"
#else 
    #define LINUX 0
    #define WINDOWS 1
    #include "plaf/plaf_win.c"
#endif

#endif
