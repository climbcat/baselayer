#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "memory.h"
#include "random.h"
#include "stuff.h"


void TestFOpen(char* filename) {
  char * buffer = 0;
  long length;
  FILE * f = fopen (filename, "rb");

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

  if (buffer)
  {
    // start to process your data / extract strings here...
  }
}

void TestRandGen() {
  RandInit();

  std::cout << "rand01():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << Rand01() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randpm1():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << RandPM1() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "rand0max(10):" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << Rand0Max(10) << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randminmax(100, 200):" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << RandMinMax(100, 200) << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randtriangle():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << RandTriangle() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randnorm():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << RandNorm() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "gaussian_double():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << RandGaussianDouble() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randminmax_i(10, 20):" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << RandMinMaxI(10, 20) << " ";
  }
  std::cout << std::endl << std::endl;
}

struct Entity {
  char* name;
  char* body;
};

void TestGPAlloc() {
  GeneralPurposeAllocator alloc( 1024 * 1024 );

  const char* s01 = "en spændende test streng";
  const char* s02 = "entity test string body indhold...";

  Entity my_entity;
  my_entity.name = (char*) alloc.Alloc(strlen(s01));
  my_entity.body = (char*) alloc.Alloc(strlen(s02));

  ListPrintSizes(alloc.blocks);

  strcpy(my_entity.name, s01);
  strcpy(my_entity.body, s02);
}

void TestWriteChars() {
  GeneralPurposeAllocator alloc( 1024 * 1024 );
  RandInit();

  int num_lines = 150;
  char* lines[num_lines];
  int line_len;

  int accum_totlen = 0;

  // fill up string locations
  for (int i = 0; i < num_lines; i++) {
    line_len = RandMinMaxI(50, 250);
    char* dest = (char*) alloc.Alloc(line_len + 1);
    WriteRandomHexStr(dest, line_len);
    lines[i] = dest;

    std::cout << dest << std::endl;
  }

  std::cout << std::endl;

  // free some of the lines
  int num_tofree = 60;
  int idx;
  for (int i = 0; i < num_tofree; i++) {

    idx = RandMinMaxI(0, num_lines - 1);
    std::cout << idx << " load: " << alloc.load << std::endl;

    bool success = alloc.Free(lines[idx]);
  }
  std::cout << std::endl << "total blocks merged:" << alloc.blocks_merged << std::endl;


  // TODO: rather visualize allocator output than typing to the terminal (pref. graphics)
}

int main (int argc, char **argv) {

  //TestFOpen();
  //TestGPAlloc();
  //TestRandGen();
  TestWriteChars();

  return 0;
}
