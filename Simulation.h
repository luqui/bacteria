#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include <sstream>
#include <fstream>
#include <stack>
#include "RandomGen.h"
#include "Vec2.h"
#include "DNA.h"
#include "Resource.h"

const double WIDTH = 64;
const double HEIGHT = 48;

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

  void dump(std::ostream& out) {
    out << "position = (" << position.x << "," << position.y << ")\n";
    out << "angle = " << angle << "\n";
    out << "energy = " << energy << "\n";
    out << "ip = " << ip << "\n";
    out << "\n";
    out << "CODE:\n";
    dna.dump(out);
  }
};


class Simulation {
 private:
  ResourceField resources;
  std::list<Organism> organisms;
  RandomGen gen;
  double regen;

 public:
  Simulation() : resources(Vec2(-WIDTH/2,-HEIGHT/2), Vec2(WIDTH/2,HEIGHT/2), (int)WIDTH, (int)HEIGHT), regen(0)
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
      Vec2 p = Vec2(gen.range(-WIDTH/2,WIDTH/2-1), gen.range(-HEIGHT/2,HEIGHT/2-1));
      Resource r = resources.take(p);
      if (r > 1) {
        resources.put(p, r-1);
      }
      else if (r == 1) {
        resources.put(p, r);
        RandomGen g = gen.split();
        add_organism(Organism(DNA::generate(gen), p, g, 2, gen.int_range(0, DNA_SIZE)));
      }

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
        if (e->key.keysym.sym == SDLK_d) {
          dump();
        }
        break;
    }
  }

  void dump() {
    std::stringstream ss;
    ss << "DUMP_" << std::time(NULL);
    std::ofstream fout(ss.str().c_str());
    for (std::list<Organism>::iterator i = organisms.begin(); i != organisms.end(); ++i) {
      i->dump(fout);
      fout << "\n-----------------------\n\n";
    }
    fout.close();

    std::cout << "Dumped to " << ss.str() << "\n";
  }

  void clamp(Vec2* v) {
    if (v->x < -WIDTH/2) { v->x = -WIDTH/2; }
    if (v->x > WIDTH/2-1) { v->x = WIDTH/2-1; }
    if (v->y < -HEIGHT/2) { v->y = -HEIGHT/2; }
    if (v->y > HEIGHT/2-1) { v->y = HEIGHT/2-1; }
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


inline void Organism::step(double dt, Simulation* sim, bool* death) {
  ResourceField& grid = sim->get_resources();

  while (true) {
    energy -= 0.002;   // small cost to thinking

    if (energy < 1) {
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

      case INSTR_METABOLIZE: {
        Resource r = grid.take(position);
        if (r != RES_NONE) {
            if (r == i.metabolize.depth && r < RES_MAX) {
                grid.put(position, r+1);
                energy += 20.0;
            }
            else {
                grid.put(position, r);
                energy -= 0.2;
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
