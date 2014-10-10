#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include "RandomGen.h"
#include "Vec2.h"
#include "DNA.h"
#include "Resource.h"

class Organism {
 private:
  DNA dna;
  Vec2 position;
  RandomGen gen;
  double energy;

 public:
  Organism(DNA dna, Vec2 position, RandomGen gen, double energy)
      : dna(dna), position(position), gen(gen), energy(energy)
  { }

  double size() const {
    return 0.1;
  }

  void step(double dt, class Simulation* sim, bool* death);

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
  RandomGen gen;

 public:
  Simulation() : resources(Vec2(-16,-12), Vec2(16,12), 64, 48)
  { }

  ResourceField& get_resources() { return resources; }

  void add_organism(const Organism& org) {
    organisms.push_front(org);
  }

  void step(double dt) {
    for (std::list<Organism>::iterator i = organisms.begin(); i != organisms.end(); ) {
      bool death = false;
      i->step(dt, this, &death);
      if (death) {
        organisms.erase(i++);
      }
      else {
        ++i;
      }
    }

    resources.step(dt, gen);
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

  void clamp(Vec2* v) {
    if (v->x < -16) { v->x = -16; }
    if (v->x > 15) { v->x = 15; }
    if (v->y < -12) { v->y = -12; }
    if (v->y > 11) { v->y = 11; }
  }
};


inline void Organism::step(double dt, Simulation* sim, bool* death) {
  position += dt * dna.speed * Vec2(gen.range(-1,1), gen.range(-1,1));
  sim->clamp(&position);

  energy -= dt * dna.speed;

  if (energy <= 1) {
    *death = true;
    return;
  }


  ResourceField& grid = sim->get_resources();

  if (energy < dna.energy_to_eat) {
    Resource r = grid.take(position);
    if (r == RES_ENERGY) {
      energy += 20;
      grid.put(position, RES_POOP);
    }
    else {
      grid.put(position, r);
    }
  }

  if (energy > dna.energy_to_split) {
    //divide
    *death = true;

    RandomGen g1, g2;
    gen.split(&g1, &g2);

    DNA dna1 = dna;
    dna1.mutate(g1);
    Organism o1(dna1, position, g1, energy/2);
    sim->add_organism(o1);

    DNA dna2 = dna;
    dna2.mutate(g2);
    Organism o2(dna2, position, g2, energy/2);
    sim->add_organism(o2);
  }

}



#endif
