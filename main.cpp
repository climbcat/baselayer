#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "SDL2/SDL.h"

#include "types.h"
#include "memory.h"
#include "random.h"
#include "frametimer.h"
#include "various.h"
#include "tests.h"


#define RGB_DEEP_BLUE 20, 50, 150
struct Color {
  u8 r = 20;
  u8 g = 50;
  u8 b = 150;
};
Color g_bckgrnd_color;
Color g_rect_color { 0, 255, 0 };

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
          g_bckgrnd_color.b = (u8) MaxI(g_bckgrnd_color.b - 10, 0);
          std::cout << (int) g_bckgrnd_color.b << std::endl;
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
          g_bckgrnd_color.b = (u8) MinI(g_bckgrnd_color.b + 10, 255);
          std::cout << (int) g_bckgrnd_color.b << std::endl;
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
  SDL_SetRenderDrawColor(renderer, g_bckgrnd_color.r, g_bckgrnd_color.g, g_bckgrnd_color.b, 255);
	SDL_RenderClear(renderer);

  SDL_Rect rect;
  rect.x = 250;
  rect.y = 150;
  rect.w = 200;
  rect.h = 200;

  SDL_SetRenderDrawColor(renderer, g_rect_color.r, g_rect_color.g, g_rect_color.b, 255);
  //SDL_RenderDrawRect(renderer, &rect);
  SDL_RenderFillRect(renderer, &rect);

  SDL_RenderPresent(renderer);
}

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

void TestOpenWindow() {
  SDL_Init(SDL_INIT_VIDEO);

  // window
  SDL_Window* window = SDL_CreateWindow(
    "Mimic",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    WINDOW_WIDTH, WINDOW_HEIGHT,
    SDL_WINDOW_OPENGL
  );

  // renderer
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  FrameTimer frame_tm(1/60);
  bool terminated = false;
  while (terminated != true) {
    if (DoInput() == -1) terminated = true;

    // TODO: do logics

    // TODO: do rendering
    DoRender(renderer);

    frame_tm.wait_for_frame();
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}


int main (int argc, char **argv) {
  RunTests();

  //TestOpenWindow();
}
