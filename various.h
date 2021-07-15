#ifndef __VARIOUS_H__
#define __VARIOUS_H__

#include <unistd.h>

#include "random.h"


void WriteRandomHexStr(char* dest, int len) {
  RandInit();
  for (int i = 0; i < len; i++) {
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
  *dest = '\0';
}



char* LoadFilePath(char* filepath) {
  char * buffer = NULL;
  long length;
  FILE * f = fopen(filepath, "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = (char*) malloc (length);
    if (buffer)
      fread (buffer, 1, length, f);
    fclose (f);
  }

  return buffer;
}


char* LoadFile(char* filename, bool use_cwd = false) {
  if (use_cwd) {
    char cwd[KILOBYTE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      strcat(cwd, "/");
      strcat(cwd, filename);
      filename = (char*) cwd;

      return LoadFilePath(filename);
    }
    return NULL;
  }
  else
    return LoadFilePath(filename);
}


#endif