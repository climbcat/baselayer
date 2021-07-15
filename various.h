#ifndef __STUFF_H__
#define __STUFF_H__


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

char* LoadFile(char* filename) {
  char * buffer = NULL;
  long length;
  FILE * f = fopen(filename, "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = (char*) malloc (length);
    if (buffer)
    {
      fread (buffer, 1, length, f);
    }
    fclose (f);
  }

  return buffer;
}


#endif