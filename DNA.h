#ifndef __DNA_H__
#define __DNA_H__

#include "RandomGen.h"

struct DNA {
  double speed;
  double energy_to_eat;
  double energy_to_split;

  void mutate(RandomGen& g) {
    speed += g.range(-0.1, 0.1);
    energy_to_eat += g.range(-1, 1);
    energy_to_split += g.range(-1, 1);
  }

  static DNA initial() {
    DNA dna;
    dna.speed = 1;
    dna.energy_to_eat = 10;
    dna.energy_to_split = 20;
    return dna;
  }
};

#endif
