#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include <list>
#include <cstdlib>


class vec2 {
 public:
  vec2(double x, double y) : x(x), y(y) { }
  double x, y;

  vec2& operator *= (double a) {
    x *= a;
    y *= a;
  }

  vec2& operator += (const vec2& v) {
    this->x += v.x;
    this->y += v.y;
  }

};

static vec2 operator* (double a, vec2 v) {
  return v *= a;
}

static vec2 operator+ (vec2 u, const vec2& v) {
  return u += v;
}


class RandomGen {
 public:
  double range(double min, double max) {
    double r = std::rand() / (double)RAND_MAX;
    return min + (max - min) * r;
  }
};


class DNA { };


class Organism {
 private:
  DNA dna;
  vec2 position;
  RandomGen gen;

 public:
  Organism(DNA dna, vec2 position, RandomGen gen)
      : dna(dna), position(position), gen(gen)
  { }

  double size() const {
    return 0.1;
  }

  void step(double dt) {
    position += dt * vec2(gen.range(-1,1), gen.range(-1,1));
  }

  void draw() {
    glPushMatrix();
    glTranslated(position.x, position.y, 0);
    glScaled(size(), size(), 1);
    glColor3d(1,1,1);
    glBegin(GL_POLYGON);
      glVertex2d(-1,-1);
      glVertex2d(-1, 1);
      glVertex2d( 1, 1);
      glVertex2d( 1,-1);
    glEnd();
    glPopMatrix();
  }
};


class Simulation {
 private:
  std::list<Organism> organisms;

 public:
  void add_organism(const Organism& org) {
    organisms.push_back(org);
  }

  void step(double dt) {
    for (std::list<Organism>::iterator i = organisms.begin(); i != organisms.end(); ++i) {
      i->step(dt);
    }
  }

  void draw() {
    for (std::list<Organism>::iterator i = organisms.begin(); i != organisms.end(); ++i) {
      i->draw();
    }
  }

  void event(SDL_Event* e) {
    switch (e->type) {
      case SDL_QUIT:
        SDL_Quit();
        std::exit(0);
        break;
    }
  }
};


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

  glScaled(1/4.0, 1/3.0, 1);

  Simulation sim;
  sim.add_organism(Organism(DNA(), vec2(0,0), RandomGen()));

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


