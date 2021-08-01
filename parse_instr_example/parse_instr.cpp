#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "parse_instr.h"


int main (int argc, char **argv) {
  char *input = argv[1];

  if (argc != 2) {
    printf("Input folder or file.\n");
    exit(0);
  }

  StackAllocator stack_files(10 * MEGABYTE);
  StackAllocator stack_work(10 * MEGABYTE);
  ArrayListT<char*> filepaths;

  bool print_detailed = false;

  if (IsInstrFile(input)) {
    filepaths.Init(stack_files.Alloc(sizeof(char*) * 1));
    filepaths.Add(&input);
    print_detailed = true;
  }
  else {
    filepaths = GetFilesInFolderPaths(input, &stack_files);
  }

  for (int i = 0; i < filepaths.len; ++i) {
    stack_work.Clear();

    char *filename = *filepaths.At(i);
    if (!IsInstrFile(filename)) {
      printf("skipping #%.3d: %s\n", i, filename);
      continue;
    }

    char *text = LoadFile(filename, false, &stack_files);
    if (text == NULL) {
      continue;
    }
    Tokenizer tokenizer = {};
    tokenizer.Init(text);

    printf("parsing  #%.3d: %s  ", i, filename);

    InstrDef instr = ParseInstrument(&tokenizer, &stack_work);
    printf("  %s\n", instr.name);

    if (print_detailed) {
      PrintInstrumentParse(instr);
      exit(0);
    }
  }
}