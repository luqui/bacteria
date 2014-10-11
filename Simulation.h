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
  std::deque<Resource> buffer;

  int ip;

 public:
  Organism(DNA dna, Vec2 position, RandomGen gen, double energy, int ip)
      : dna(dna), gen(gen), position(position), energy(energy), ip(ip)
  {
    angle = gen.range(0, 2*M_PI);
  }

  Vec2 get_position() const { return position; }

  double size() const {
    return 0.1;
  }

  bool find_resource(Resource r) {
    for (std::deque<Resource>::iterator i = buffer.begin(); i != buffer.end(); ++i) {
      if (*i == r) {
        buffer.erase(i);
        return true;
      }
    }
    return false;
  }

  void drop_buffer(class Simulation* sim);
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
  Simulation() : resources(Vec2(-32,-24), Vec2(32,24), 64, 48), regen(0)
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
      Vec2 p = Vec2(gen.range(-32,-12), gen.range(-24,-4));
      RandomGen g = gen.split();
      add_organism(Organism(DNA::generate(gen), p, g, 2, gen.int_range(0, DNA_SIZE)));

      regen += -std::log(1 - gen.range(0,1)) / 20;
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
      case SDL_KEYDOWN:
        if (e->key.keysym.sym == SDLK_SPACE) {
          kill_eden();
        }
        break;
    }
  }

  void clamp(Vec2* v) {
    if (v->x < -32) { v->x = -32; }
    if (v->x > 31) { v->x = 31; }
    if (v->y < -24) { v->y = -24; }
    if (v->y > 23) { v->y = 23; }
  }

  void kill_eden() {
    for (std::list<Organism>::iterator i = organisms.begin(); i != organisms.end();) {
      Vec2 p = i->get_position();
      if (p.x < -12 && p.y < -4) {
        organisms.erase(i++);
      }
      else {
        ++i;
      }
    }
  }
};


inline void Organism::drop_buffer(Simulation* sim) {
  ResourceField& grid = sim->get_resources();
  while (!buffer.empty()) {
    grid.put(position, buffer.back());
    buffer.pop_back();
  }
}

inline void Organism::step(double dt, Simulation* sim, bool* death) {
  ResourceField& grid = sim->get_resources();

  while (true) {
    energy -= 0.001;   // small cost to thinking

    if (energy < 1) {
      drop_buffer(sim);
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
        energy -= std::abs(i.forward.speed) * dt;
        return;
      }

      case INSTR_ROTATE: {
        angle += dt * i.rotate.speed;
        break;
      }

      case INSTR_ABSORB: {
        Resource r = grid.take(position);
        if (r != RES_NONE) {
          buffer.push_back(r);
        }
        return;
      }

      case INSTR_EXCRETE: {
        if (!buffer.empty()) {
          grid.put(position, buffer.back());
          buffer.pop_back();
        }
        break;
      }

      case INSTR_METABOLIZE_ENERGY: {
        // dessert is toxic to energy metabolizers
        if (find_resource(RES_DESSERT)) {
          drop_buffer(sim);
          grid.put(position, RES_DESSERT);
          *death = true;
          return;
        }

        if (find_resource(RES_ENERGY)) {
          grid.put(position, RES_POOP);
          energy += 20;
        }
        return;
      }

      case INSTR_METABOLIZE_POOP: {
        if (find_resource(RES_POOP)) {
          if (find_resource(RES_POOP)) {
            energy += 20;
            grid.put(position, RES_DESSERT);
          }
          else {
            buffer.push_back(RES_POOP);
          }
        }
        return;
      }

      case INSTR_METABOLIZE_DESSERT: {
        if (find_resource(RES_DESSERT)) {
          energy += 20;
        }
        return;
      }

      case INSTR_DIVIDE: {
        //divide
        drop_buffer(sim);
        *death = true;

        RandomGen g1 = gen.split();
        RandomGen g2 = gen.split();

        DNA dna1 = dna;
        dna1.mutate(g1);
        Organism o1(dna1, position, g1, energy/2, ip);
        sim->add_organism(o1);

        DNA dna2 = dna;
        dna2.mutate(g2);
        Organism o2(dna2, position, g2, energy/2, ip);
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
