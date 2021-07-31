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
#include "parse_slconfig.h"
#include "parse_mcstas.h"


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
  while (arena.max_size - arena.used > len) {
    std::cout << "used: " << arena.used << " total: " << arena.max_size <<  std::endl;

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

      case TOK_INT: {
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


void TestParseNumerics() {

  Tokenizer tokenizer = {};
  tokenizer.at = (char*) "1.222\n666\n1e49\n5E1\n45.6353573573573\n32123.23245.2\n3562346.123123e0908\0\0";
  // should output: float, int, sci, sci, float, (float, dot, int), sci
 
  bool parsing = true;
  while (parsing) {
    Token token = GetToken(&tokenizer);
    TokenTypePrint(token.type, false);
    printf(" --- %.*s\n", token.len, token.text);

    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
      } break;

      case TOK_DASH: {
      } break;

      case TOK_IDENTIFIER: {
      } break;

      default: {
      } break;
    }
  }

}


void TestParseNumTokenSeparatedStuff() {

  const char *string_24 =
    " \
    double mono_q = 1.8734; \
    double OMA; \
    double RV; \
    double y_mono = 0.025; \
    double NV = 5; \
    double d_phi_0; \
    double TTM; \
    double sample_radius = 0.008/2; \
    double sample_height = 0.03; \
    double can_radius = 0.0083/2; \
    double can_height = 0.0303; \
    double can_thick = 0.00015; \
    /******Mirrorvalues*****/  \
    double alpha; \
    double Qc=0.0217; \
    double R0=0.995; \
    double Mvalue=1.9; \
    double W=1.0/250.0; \
    double alpha_curve; \
    double Qc_curve=0.0217; \
    double R0_curve= 0.995; \
    double Mvalue_curve=2.1; \
    double W_curve=1.0/250.0; \
    double ldiff=0.05; \
    /* Curved guide element angle*/ \
    double angleGuideCurved; \
    ";

  u32 count = CountTokenSeparatedStuff((char*) string_24, TOK_SEMICOLON, TOK_ENDOFSTREAM);
  printf("num1 = %d\n", count);
  assert( count == 24 );

  const char *string_1 = 
    " \
    double mono_q = 1.8734; \
    ";

  count = CountTokenSeparatedStuff((char*) string_1, TOK_SEMICOLON, TOK_ENDOFSTREAM);
  printf("num2 = %d\n", count);
  assert( count == 1 );

  const char *string_0 = 
    " \
    ";

  count = CountTokenSeparatedStuff((char*) string_0, TOK_SEMICOLON, TOK_ENDOFSTREAM);
  printf("num3 = %d\n", count);
  assert( count == 0 );

  const char *string_end = 
    " \
    yheight=0.156, xwidth=0.126, \
    Lmin=lambda-ldiff/2,Lmax=lambda+ldiff/2, \
    dist=1.5, focus_xw = 0.02, focus_yh = 0.12) \
    Lmin=lambda-ldiff/2,Lmax=lambda+ldiff/2, \
    ";

  count = CountTokenSeparatedStuff((char*) string_end, TOK_COMMA, TOK_RBRACK);
  printf("num4 = %d\n", count);
  assert( count == 7 );
}

void TestParseStructMembers() {

  const char *text =
    "\
    double mono_q = 1.8734, mono_q1, mono_q2 = 2.0, mono_q3;\n\
    double OMA;\n\
    double RV;\n\
    double y_mono = 0.025;\n\
    double NV = 5;\n\
    double d_phi_0;\n\
    double TTM;\n\
    double sample_radius = 0.008/2;\n\
    double sample_height = 0.03;\n\
    double can_radius = 0.0083/2;\n\
    double can_height = 0.0303;\n\
    double can_thick = 0.00015;\n\
    /******Mirrorvalues*****/\n\
    double alpha;\n\
    double Qc=0.0217;\n\
    double R0=0.995;\n\
    double Mvalue=1.9;\n\
    double W=1.0/250.0;\n\
    double alpha_curve;\n\
    double Qc_curve=0.0217;\n\
    double R0_curve= 0.995;\n\
    double Mvalue_curve=2.1;\n\
    double W_curve=1.0/250.0;\n\
    double ldiff=0.05;\n\
    /* Curved guide element angle*/\n\
    double angleGuideCurved;\n\
    ";
  StackAllocator stack(MEGABYTE);
  Tokenizer tokenizer = {};
  tokenizer.Init( (char*) text );
  ArrayListT<StructMember> decls = ParseStructMembers(&tokenizer, &stack);

  printf("declare members:\n");
  for (int i = 0; i < decls.len; ++i) {
    StructMember *memb = decls.At(i);
    printf("  %s %s = %s\n", memb->type, memb->name, memb->defval);
  }
  printf("\n");
}

void TestTokenizerLineNums() {
  const char *string = 
    " \
    yheight=0.156, xwidth=0.126, \n\
    Lmin=lambda-ldiff/2,Lmax=lambda+ldiff/2, \n\
    dist=1.5, focus_xw = 0.02, focus_yh = 0.12) \n\
    Lmin=lambda-ldiff/2,Lmax=lambda+ldiff/2, \n\
    ";

  Tokenizer tokenizer;
  tokenizer.Init((char*) string);
  printf("line0 = %d\n", tokenizer.line);

  Token token;
  token.type = TOK_UNKNOWN;
  while (token.type != TOK_ENDOFSTREAM) {
    token = GetToken(&tokenizer);
  }

  printf("linen = %d\n", tokenizer.line);
  assert( tokenizer.line == 5 );
}

void TestTokenizerLineNums_PSI() {
  char *filename = (char*) "PSI.instr";
  char *text = LoadFile(filename, true);
  if (text == NULL) {
      printf("could not load file %s\n", filename);
      exit(1);
  }
  printf("parsing file %s\n\n", filename);

  StackAllocator stack(MEGABYTE);

  Tokenizer tokenizer = {};
  tokenizer.Init(text);

  ParseInstrument(&tokenizer, &stack);

  u32 num_endofline = CountNumCharsInText(text, '\n');
  printf("\n\n");
  printf("text:      %d\n", num_endofline);
  printf("tokenizer: %d\n", tokenizer.line);

  assert( tokenizer.line = num_endofline );
}

void TestGetFilenamesInFolder() {
  char *folder = (char*) "/usr/share/mcstas/3.0-dev/examples";
  StackAllocator stack(MEGABYTE);

  ArrayListT<char*> filepaths = GetFilesInFolderPaths(folder, &stack);

  for (int i = 0; i < filepaths.len; ++i) {
    printf("%s\n", *filepaths.At(i));
  }

}

void TestParseUntilEndOfExpression() {
  const char *string = 
    "d_phi=RAD2DEG*atan2(0.5,L3),";

  Tokenizer tokenizer;
  tokenizer.Init((char*) string);
  Token token;

  if (!RequireToken(&tokenizer, &token, TOK_IDENTIFIER)) assert(1 == 0);
  if (!RequireToken(&tokenizer, &token, TOK_ASSIGN)) assert(2 == 0);
  assert(ParseExpression(&tokenizer, &token) == true);
  token.PrintValue();
  if (!RequireToken(&tokenizer, &token, TOK_COMMA)) assert(4 == 0);
}

void TestCountCommaSeparatedStuff() {
  char *text = (char*) "\
    (lambda = 2.5666,\n\
    R = 0.87 and a bunc of stuff(),\n\
    string filename = \"Na2Ca3Al2F14.laz\",\n\
    D_PHI=6,\n\
    SHIFT=0,\n\
    PACK=0.7,\n\
    Dw=0.8,\n\
    BARNS=1)\n\
  ";

  Tokenizer tokenizer;
  tokenizer.Init(text);
  Token token;

  if (!RequireToken(&tokenizer, &token, TOK_LBRACK)) exit(1);

  u32 count = CountCommaSeparatedSequenceOfExpresions(tokenizer.at);
  printf("%d\n", count);
}

void RunTests(int argc, char **argv) {
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
  //TestParseConfig();
  //TestParseNumerics();
  //TestParseNumTokenSeparatedStuff();
  //TestTokenizerLineNums();
  //TestTokenizerLineNums_PSI();
  //TestGetFilenamesInFolder();
  //TestParseStructMembers();
  //TestParseUntilEndOfExpression();
  //TestCountCommaSeparatedStuff();

  //TestParseMcStasInstr(argc, argv);
  TestParseMcStasInstrExamplesFolder(argc, argv);
}


#endif