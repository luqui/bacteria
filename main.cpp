#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include <list>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <cmath>


struct Vec2 {
  Vec2(double x, double y) : x(x), y(y) { }
  double x, y;

  Vec2& operator *= (double a) {
    x *= a;
    y *= a;
  }

  Vec2& operator += (const Vec2& v) {
    this->x += v.x;
    this->y += v.y;
  }

  Vec2& operator -= (const Vec2& v) {
    this->x -= v.x;
    this->y -= v.y;
  }
};

static Vec2 operator * (double a, Vec2 v) {
  return v *= a;
}

static Vec2 operator + (Vec2 u, const Vec2& v) {
  return u += v;
}

static Vec2 operator - (Vec2 u, const Vec2& v) {
  return u -= v;
}


struct Color {
  double r, g, b;
  Color(double r, double g, double b) : r(r), g(g), b(b) { }

  void set() {
    glColor3d(r,g,b);
  }
};


class RandomGen {
 public:
  double range(double min, double max) {
    double r = std::rand() / (double)RAND_MAX;
    return min + (max - min) * r;
  }
};


enum Resource { RES_NONE, RES_ENERGY, RES_POOP };

class ResourceField {
 private:
  Vec2 ll, ur;
  int dim_x, dim_y;

  std::vector< std::vector<Resource> > grid;

  bool to_grid(Vec2 position, int* x, int* y) {
    *x = int(round(dim_x * ((position.x - ll.x) / (ur.x - ll.x))));
    *y = int(round(dim_y * ((position.y - ll.y) / (ur.y - ll.y))));
    return 0 <= *x && *x < dim_x && 0 <= *y && *y < dim_y;
  }

 public:
  ResourceField(Vec2 ll, Vec2 ur, int dim_x, int dim_y)
      : ll(ll), ur(ur), dim_x(dim_x), dim_y(dim_y),
        grid(dim_x, std::vector<Resource>(dim_y))
  {
    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        grid[i][j] = RES_ENERGY;
      }
    }
  }

  static Color resource_color(Resource res) {
    switch (res) {
      case RES_NONE: return Color(0,0,0);
      case RES_ENERGY: return Color(0.2,0.2,0);
      case RES_POOP: return Color(0.2,0.1,0);
      default: abort();
    }
  }

  void draw() {
    double dx = (ur.x - ll.x) / dim_x;
    double dy = (ur.y - ll.y) / dim_y;

    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        Vec2 p = ll + Vec2(i * dx, j * dy);
        resource_color(grid[i][j]).set();
        glBegin(GL_POLYGON);
          glVertex2d(p.x - dx/2, p.y - dy/2);
          glVertex2d(p.x - dx/2, p.y + dy/2);
          glVertex2d(p.x + dx/2, p.y + dy/2);
          glVertex2d(p.x + dx/2, p.y - dy/2);
        glEnd();
      }
    }
  }

  Resource take(Vec2 position) {
    int x, y;
    if (to_grid(position, &x, &y)) {
      Resource r = grid[x][y];
      grid[x][y] = RES_NONE;
      return r;
    }
    else {
      return RES_NONE;
    }
  }

  bool put(Vec2 position, Resource r) {
    int x, y;
    if (to_grid(position, &x, &y) && grid[x][y] == RES_NONE) {
      grid[x][y] = r;
      return true;
    }
    else {
      return false;
    }
  }
};


class DNA { };


class Organism {
 private:
  DNA dna;
  Vec2 position;
  RandomGen gen;
  double energy;

 public:
  Organism(DNA dna, Vec2 position, RandomGen gen)
      : dna(dna), position(position), gen(gen), energy(0)
  { }

  double size() const {
    return 0.1;
  }

  void step(double dt, ResourceField* grid, bool* death) {
    position += dt * Vec2(gen.range(-1,1), gen.range(-1,1));

    energy -= dt;
    if (energy < 10) {
      Resource r = grid->take(position);
      if (r == RES_ENERGY) {
        energy += 20;
        assert(grid->put(position, RES_POOP));
      }
      else {
        assert(grid->put(position, r));
      }
    }

    if (energy <= 0) {
      *death = true;
    }
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
  ResourceField resources;
  std::list<Organism> organisms;

 public:
  Simulation() : resources(Vec2(-16,-12), Vec2(16,12), 32, 24) 
  { }

  void add_organism(const Organism& org) {
    organisms.push_back(org);
  }

  void step(double dt) {
    for (std::list<Organism>::iterator i = organisms.begin(); i != organisms.end(); ) {
      bool death = false;
      i->step(dt, &resources, &death);
      if (death) {
        organisms.erase(i++);
      }
      else {
        ++i;
      }
    }
  }

  void draw() {
    resources.draw();
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

  glScaled(1/16.0, 1/12.0, 1);

  Simulation sim;
  sim.add_organism(Organism(DNA(), Vec2(0,0), RandomGen()));

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


