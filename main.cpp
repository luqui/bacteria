#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include <list>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <cmath>
#include <ctime>

#include "Vec2.h"
#include "Color.h"
#include "RandomGen.h"
#include "Resource.h"
#include "DNA.h"
#include "Simulation.h"

int main(int argc, char** argv) {
  std::srand(std::time(NULL));

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Surface* surface = SDL_SetVideoMode(800, 600, 24, SDL_OPENGL);
  if (surface == NULL) {
    std::cerr << "Failed to initialize video mode: " << SDL_GetError() << std::endl;
    return 1;
  }

  glScaled(1/16.0, 1/12.0, 1);

  Simulation sim;
  {
    Organism o(DNA::initial(), Vec2(0,0), RandomGen(), 10);
    sim.add_organism(o);
  }

  double dt = 1/30.0f;
  while(true) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      sim.event(&e);
    }

    sim.step(dt);

    glClear(GL_COLOR_BUFFER_BIT);
    sim.draw();
    SDL_GL_SwapBuffers();

    SDL_Delay(1000*dt);
  }

  SDL_Quit();
}


