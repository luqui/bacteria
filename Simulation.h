#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include <stack>
#include "RandomGen.h"
#include "Vec2.h"
#include "DNA.h"
#include "Resource.h"

class Organism {
 private:
  DNA dna;
  RandomGen gen;
  Vec2 position;
  double angle;
  double energy;
  std::stack<Resource> buffer;

  int ip;

 public:
  Organism(DNA dna, Vec2 position, RandomGen gen, double energy, int ip)
      : dna(dna), gen(gen), position(position), energy(energy), ip(ip)
  {
    angle = gen.range(0, 2*M_PI);
  }

  double size() const {
    return 0.1;
  }

  void step(double dt, class Simulation* sim, bool* death);

  void draw() {
    glPushMatrix();
    glTranslated(position.x, position.y, 0);
    glScaled(size(), size(), 1);
    dna.pigment.set();
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
  double regen;

 public:
  Simulation() : resources(Vec2(-16,-12), Vec2(16,12), 64, 48), regen(0)
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

    regen -= dt;
    while (regen < 0) {
      Vec2 p = Vec2(gen.range(-10,10), gen.range(-8,8));
      RandomGen g = gen.split();
      add_organism(Organism(DNA::generate(gen), p, g, 10, gen.int_range(0, DNA_SIZE)));

      regen += -std::log(1 - gen.range(0,1));
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

  void clamp(Vec2* v) {
    if (v->x < -16) { v->x = -16; }
    if (v->x > 15) { v->x = 15; }
    if (v->y < -12) { v->y = -12; }
    if (v->y > 11) { v->y = 11; }
  }
};


inline void Organism::step(double dt, Simulation* sim, bool* death) {
  ResourceField& grid = sim->get_resources();

  while (true) {
    energy -= 0.001;   // small cost to thinking

    if (energy < 1) {
      while (!buffer.empty()) {
        grid.put(position, buffer.top());
        buffer.pop();
      }
      *death = true;
      return;
    }

    Instruction& i = dna.code[ip];
    ip = i.next;
    switch (i.type) {
      case INSTR_IDLE: {
        return;
      }

      case INSTR_FORWARD: {
        position += dt * i.forward.speed * Vec2(std::cos(angle), std::sin(angle));
        sim->clamp(&position);
        energy -= i.forward.speed * dt;
        return;
      }

      case INSTR_ROTATE: {
        angle += dt * i.rotate.speed;
        break;
      }

      case INSTR_ABSORB: {
        Resource r = grid.take(position);
        if (r != RES_NONE) {
          buffer.push(r);
        }
        return;
      }

      case INSTR_EXCRETE: {
        if (!buffer.empty()) {
          grid.put(position, buffer.top());
          buffer.pop();
        }
        break;
      }

      case INSTR_METABOLIZE: {
        if (!buffer.empty()) {
          Resource r = buffer.top();
          buffer.pop();

          if (r == RES_ENERGY) {
            grid.put(position, RES_POOP);
            energy += 20;
          }
          else {
            grid.put(position, r);
          }
        }
        return;
      }

      case INSTR_METABOLIZE2: {
        if (buffer.size() >= 2) {
          Resource r = buffer.top();
          buffer.pop();

          Resource r2 = buffer.top();
          buffer.pop();

          if (r == RES_POOP && r2 == RES_POOP) {
            grid.put(position, RES_ENERGY);
          }
          else {
            grid.put(position, r2);
            buffer.push(r);
          }
        }
        return;
      }

      case INSTR_DIVIDE: {
        //divide
        *death = true;

        RandomGen g1 = gen.split();
        RandomGen g2 = gen.split();

        DNA dna1 = dna;
        dna1.mutate(g1);
        Organism o1(dna1, position, g1, energy/2, ip);
        sim->add_organism(o1);

        DNA dna2 = dna;
        dna2.mutate(g2);
        Organism o2(dna2, position, g2, energy/2, i.divide.child_ip);
        sim->add_organism(o2);
        return;
      }

      case INSTR_CMP_ENERGY: {
        if (energy >= i.cmp_energy.threshold) {
          ip = i.cmp_energy.greater;
        }
        break;
      }

      case INSTR_RANDOM_BRANCH: {
        if (gen.range(0,1) < i.random_branch.probability) {
          ip = i.random_branch.branch;
        }
        break;
      }

      default: {
        abort();
      }
    }
  }
}



#endif
