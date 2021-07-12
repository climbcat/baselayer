#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "memory.h"
#include "random.h"


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
  rand_init();

  std::cout << "rand01():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << rand01() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randpm1():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << randpm1() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "rand0max(10):" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << rand0max(10) << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randminmax(100, 200):" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << randminmax(100, 200) << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randtriangle():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << randtriangle() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randnorm():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << randnorm() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "gaussian_double():" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << gaussian_double() << " ";
  }
  std::cout << std::endl << std::endl;

  std::cout << "randminmax_i(10, 20):" << std::endl;
  for (int i = 0; i<20; i++) {
    std::cout << randminmax_i(10, 20) << " ";
  }
  std::cout << std::endl << std::endl;
}

void TestGPAlloc() {
  GeneralPurposeAllocator alloc( 1024 * 1024 );

  const char* s01 = "en spændende test streng";
  const char* s02 = "entity test string body indhold...";

  Entity my_entity;
  my_entity.name = (char*) alloc.alloc(strlen(s01));
  my_entity.body = (char*) alloc.alloc(strlen(s02));

  list_print_sizes(alloc.blocks);

  strcpy(my_entity.name, s01);
  strcpy(my_entity.body, s02);
}

int main (int argc, char **argv) {

  //TestFOpen();
  //TestGPAlloc();
  TestRandGen();

  return 0;
}
