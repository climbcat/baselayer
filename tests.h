#ifndef __TESTS_H__
#define __TESTS_H__


#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "memory.h"
#include "random.h"
#include "various.h"


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
  GeneralPurposeAllocator alloc( MEGABYTE );

  const char* s01 = "en spændende test streng";
  const char* s02 = "entity test string body indhold...";

  Entity my_entity;
  my_entity.name = (char*) alloc.Alloc(strlen(s01));
  my_entity.body = (char*) alloc.Alloc(strlen(s02));

  ListPrintSizes(alloc.blocks);

  strcpy(my_entity.name, s01);
  strcpy(my_entity.body, s02);
}


void TestIterateAndDisplayGPAList(GeneralPurposeAllocator* alloc) {
  MemoryBlock* lst = alloc->blocks;
  int idx = 0;
  do {
    std::cout << idx << " size: " << lst->total_size;

    if (lst->used == true)
      std::cout << " used" << std::endl;
    else
      std::cout << " not used" << std::endl;

    idx++;
    lst = lst->next;
  } while (lst != alloc->blocks);
}


void TestGPAlloc2WriteChars() {
  GeneralPurposeAllocator alloc( MEGABYTE );
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

  TestIterateAndDisplayGPAList(&alloc);
}


void TestGPAOccupancy() {
  GeneralPurposeAllocator alloc( MEGABYTE );
  RandInit();

  // TODO: compress such temp allocation into convenient memetic form
  u32 stack_size = MEGABYTE / 50 * sizeof(char**);
  StackAllocator stack( stack_size );
  char** lines = (char**) stack.Alloc( stack_size );
  u32 lines_idx = 0;

  // fill up the GPA
  u32 line_len = 101;
  char* dest = (char*) alloc.Alloc(line_len);
  do {
    WriteRandomHexStr(dest, line_len);

    std::cout
      << lines_idx << " load: " << alloc.load
      << " len: " << ListLen(alloc.blocks)
      << std::endl;
    

    // TODO: compress such vector usage into convenient memetic form
    *(lines + lines_idx) = dest;
    lines_idx++;

    if (lines_idx == 22)
      std::cout << std::endl;

    line_len = RandMinMaxI(74, 75);
    dest = (char*) alloc.Alloc(line_len + 1);
  } while (dest != NULL);


  std::cout << "total items allocated: " << lines_idx << std::endl;
}


struct PoolAllocTestStruct {
  int some_num;
  char some_word[35];
};

void TestPoolAlloc() {
  int batch_size = 10;
  PoolAllocator palloc(sizeof(PoolAllocTestStruct), batch_size);

  void* ptrs[batch_size];
  std::cout << "pool load: " << palloc.load << " element size: " << sizeof(PoolAllocTestStruct) << std::endl;

  // allocate items
  for (int i = 0; i < batch_size; i++) {
    ptrs[i] = palloc.Get();
    int relloc = (u8*) ptrs[i] - (u8*) palloc.root;
    std::cout << "pool load: " << palloc.load << " - relloc: " << relloc << std::endl;
  }

  // free four items at stride 2
  for (int i = 0; i < batch_size - 2; i += 2) {
    palloc.Release(ptrs[i]);
    std::cout << "pool load: " << palloc.load << std::endl;
  }

  // put them back in
  for (int i = 0; i < batch_size - 2; i += 2) {
    ptrs[i] = palloc.Get();
    std::cout << "pool load: " << palloc.load << std::endl;
  }

  // it is full now
  assert(palloc.Get() == NULL);

  palloc.Release(ptrs[4]);
  std::cout << "pool load: " << palloc.load << std::endl;
  palloc.Release(ptrs[5]);
  std::cout << "pool load: " << palloc.load << std::endl;

  ptrs[5] = palloc.Get();
  std::cout << "pool load: " << palloc.load << std::endl;

  ptrs[4] = palloc.Get();
  std::cout << "pool load: " << palloc.load << std::endl;

  assert(palloc.Get() == NULL);
}

void TestStackAlloc() {
  StackAllocator arena(SIXTEEN_K);
  std::cout << "initial arena used: " << arena.used << std::endl;

  char* strings[1000];
  int sidx = 0;

  // allocate a bunch of strings
  char* dest;
  int len = 0;
  RandInit();
  while (arena.total_size - arena.used > len) {
    std::cout << "used: " << arena.used << " total: " << arena.total_size <<  std::endl;

    dest = (char*) arena.Alloc(len);
    WriteRandomHexStr(dest, len);
    strings[sidx] = dest;
    sidx++;

    len = RandMinMaxI(50, 350);
  }

  std::cout << std::endl << "now printing all of those allocated strings:" << std::endl;;

  // print those strings
  for (int i = 0; i < sidx; i++) {
    std::cout << strings[i] << std::endl;
  }

  std::cout << std::endl << "unraveling down to the five first strings..." << std::endl;;

  // unravel the stack from the top, almost all of the way down (Free will actually memset to zero)
  for (int i = sidx - 1; i >= 5; i--) {
    arena.Free(strings[i]);
  }

  std::cout << std::endl << "printing those five strings:" << std::endl;;

  // print those strings
  for (int i = 0; i < 5; i++) {
    std::cout << strings[i] << std::endl;
  }

  std::cout << std::endl << "done." << std::endl;;
}

void TestLoadFile() {
  char* filename = (char*) "memory.h";
  char* text = LoadFile(filename, true);

  std::cout << text << std::endl;
}


void RunTests() {
  //TestRandGen();
  //TestLoadFile();

  //TestPoolAlloc();
  //TestStackAlloc();
  //TestGPAlloc();
  //TestGPAlloc2WriteChars();
  TestGPAOccupancy();
}

#endif