#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "memory.h"
#include "random.h"
#include "stuff.h"
#include "SDL.h"


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

void TestLoadFile() {
  char* filename = "/home/jaga/documents/gpalloc.c";
  char* text = LoadFile(filename);

  std::cout << text << std::endl;
}

#include "frametimer.h"




#define RGB_DEEP_BLUE 20, 50, 150

struct Color {
  u8 r = 20;
  u8 g = 50;
  u8 b = 150;
};
Color g_color;

int MaxI(int a, int b) {
  if (a >= b)
    return a;
  else
    return b;
}
int MinI(int a, int b) {
  if (a <= b)
    return a;
  else
    return b;
}

int DoInput(void)
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
				break;
				
			case SDL_KEYDOWN:
        if(event.key.keysym.scancode == SDL_SCANCODE_Q)
          return -1;
        if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
          g_color.b = (u8) MaxI(g_color.b - 10, 0);
          std::cout << (int) g_color.b << std::endl;
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
          g_color.b = (u8) MinI(g_color.b + 10, 255);
          std::cout << (int) g_color.b << std::endl;
        }
				break;
			case SDL_KEYUP:
				break;

			default:
				break;
		}
	}
  return 0;
}

void DoRender(SDL_Renderer* renderer) {
  SDL_SetRenderDrawColor(renderer, g_color.r, g_color.g, g_color.b, 255);
	SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
}


void TestOpenWindow() {
  SDL_Init(SDL_INIT_VIDEO);

  // window
  SDL_Window* window = SDL_CreateWindow(
    "Mimic",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    640, 480,
    0
  );

  // renderer
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  // ...
  SDL_SetRenderDrawColor(renderer, RGB_DEEP_BLUE, 255);
	SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  FrameTimer frame_tm(1/60);
  bool terminated = false;
  while (terminated != true) {
    // TODO: do input
    if (DoInput() == -1) {
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      exit(0);
    }

    // TODO: do logics

    // TODO: do rendering
    DoRender(renderer);

    frame_tm.wait_for_frame();
  }

  // Close and destroy the window
  SDL_DestroyWindow(window);

  // Clean up
  SDL_Quit();
}


int main (int argc, char **argv) {

  //TestFOpen();
  //TestGPAlloc();
  //TestRandGen();
  //TestWriteChars();
  //TestLoadFile();



  TestOpenWindow();

  
  return 0;
}
