#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>

int main() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }
  
  SDL_Surface* surface = SDL_SetVideoMode(800, 600, 24, SDL_OPENGL);
  if (surface == NULL) {
    std::cerr << "Failed to initialize video mode: " << SDL_GetError() << std::endl;
    return 1;
  }


  SDL_Quit();
}
