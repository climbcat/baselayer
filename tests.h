#ifndef __TESTS_H__
#define __TESTS_H__


#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "memory.h"
#include "random.h"
#include "various.h"
#include "perftimer.h"
#include "token.h"
#include "parsers.h"


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


void TestGPAOccupancy() {
  #define ALLOCATOR_METHOD 0
  u32 total_alloc = SIXTEEN_M;
  //u32 total_alloc = MEGABYTE;
  //u32 total_alloc = SIXTEEN_K;
  //u32 total_alloc = SIXTYFOUR_K;


  #if ALLOCATOR_METHOD == 1
  StackAllocator alloc( total_alloc );
  #endif
  #if ALLOCATOR_METHOD == 2
  GeneralPurposeAllocator alloc( total_alloc );
  #endif

  RandInit();
  StackAllocator stack( total_alloc / 50 * sizeof(char*) );
  ArrayList list(stack.AllocOpenEnded(), sizeof(char*));

  // time it
  StartTimer();

  // fill up the allocator
  u32 next_line_len = 101;

  #if ALLOCATOR_METHOD == 0
  char* dest = (char*) malloc(next_line_len + 1);
  #else
  char* dest = (char*) alloc.Alloc(next_line_len + 1);
  #endif

  u32 accum = 0;
  u32 num_blocks = 0;
  do {
    //WriteRandomHexStr(dest, next_line_len);
    list.Add(&dest);

    next_line_len = RandMinMaxI(110, 111);
    accum += next_line_len;
    num_blocks++;

    #if ALLOCATOR_METHOD == 0
    // malloc block
    dest = (char*) malloc(next_line_len + 1);
    if (accum > total_alloc)
      dest = NULL;
    #else
    dest = (char*) alloc.Alloc(next_line_len + 1);
    #endif

  } while (dest != NULL);

  StopTimer(true);
  StartTimer();

  std::cout << "freeing ... " << std::endl;
  for (int i = list.len - 1; i >= 0; i--)
    #if ALLOCATOR_METHOD == 0
    free(((char**) list.lst)[i]);
    #else
    alloc.Free(((char**) list.lst)[i]);
    #endif

  StopTimer(true);

  #if ALLOCATOR_METHOD != 0
  std::cout << "total items allocated: " << num_blocks << std::endl;
  #endif
}


struct ArrayListTestStruct {
  u32 id;
  char name[10];
  u8* next;
};


void TestArrayList() {
  StackAllocator alloc( KILOBYTE );
  ArrayList list(alloc.AllocOpenEnded(), sizeof(ArrayListTestStruct));

  // put some elements into the list
  ArrayListTestStruct s1;
  for (int i = 0; i < 10; i++) {
    s1.id = i;
    strcpy(s1.name, "hest");
    s1.next = NULL;

    list.Add(&s1);
  }

  auto lst = (ArrayListTestStruct*) list.lst; // we could do with a templated version ... but not C compatible !

  // print all
  std::cout << std::endl;
  for (int i = 0; i < list.len; i++)
    std::cout << lst[i].id << ": " << lst[i].name << std::endl;

  strcpy(s1.name, "test");
  s1.id = 1000;
  list.Insert(&s1, 4);
  list.Insert(&s1, 4);
  list.Insert(&s1, 4);

  // print all
  std::cout << std::endl;
  for (int i = 0; i < list.len; i++)
    std::cout << lst[i].id << ": " << lst[i].name << std::endl;

  list.Remove(0);
  list.Remove(1);
  list.Remove(3);
  list.Remove(6);

  // print all
  std::cout << std::endl;
  for (int i = 0; i < list.len; i++)
    std::cout << lst[i].id << ": " << lst[i].name << std::endl;
}


void TestPerfTimer() {
  StartTimer();
  Sleep(300);
  std::cout << "method 1: " << StopTimer() << " µs" << std::endl;

  PerfTimerScoped tm; // <-- the simplest way with scoped timer
  Sleep(300);
  std::cout << "method 2: " << tm.GetTimeMicroS() << " µs" << std::endl;

  std::cout << "method 3: ";
}


void TestWriteRandomStr() {
  char word[5];
  WriteRandomHexStr(word, 4, true);
  // should output 4 chars:
  std::cout << word << std::endl;
}


void TestTokenizer() {
  char* filename = (char*) "token.h";
  char* text = LoadFile(filename, true);
  if (text == NULL) {
    printf("could not load file");
    exit(1);
  }

  Tokenizer tokenizer = {};
  tokenizer.at = text;

  u32 DB_idx = 0;

  bool parsing = true;
  while (parsing) {
    Token token = GetToken(&tokenizer);

    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        printf("%d TOK_ENDOFSTREAM\n", DB_idx);

        parsing = false;
      } break;

      case TOK_UNKNOWN: {
        printf("%d TOK_UNKNOWN: %.1s\n", DB_idx, token.text);

        // skip this token
      } break;

      case TOK_IDENTIFIER: {
        printf("%d TOK_IDENTIFIER: %.*s\n", DB_idx, token.len, token.text);

        // action code here
      } break;

      case TOK_NUMERIC: {
        //printf("%d TOK_NUMERIC: %.*s\n", db_idx, token.len, token.text);

        // action code here
      } break;

      case TOK_STRING: {
        printf("%d TOK_STRING: %.*s\n", DB_idx, token.len, token.text);

        // action code here
      } break;

      case TOK_CHAR: {
        printf("%d TOK_CHAR: %.*s\n", DB_idx, token.len, token.text);

        // action code here
      } break;

      // etc. ...
    }

    ++DB_idx;
  }
}


void RunTests() {
  //TestRandGen();
  //TestLoadFile();
  //TestPerfTimer();
  //TestWriteRandomStr();

  //TestPoolAlloc();
  //TestStackAlloc();
  //TestGPAlloc();
  //TestGPAlloc2WriteChars();
  //TestGPAOccupancy();

  //TestArrayList();
  //TestTokenizer();
  TestParseConfig();
}


#endif