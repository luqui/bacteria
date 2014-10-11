#ifndef __RANDOMGEN_H__
#define __RANDOMGEN_H__

class RandomGen {
 public:
  double range(double min, double max) {
    double r = std::rand() / (double)RAND_MAX;
    return min + (max - min) * r;
  }

  int int_range(int min, int max) {
    return min + std::rand() % (max - min);
  }

  void split(RandomGen* g1, RandomGen* g2) {
    *g1 = *this;
    *g2 = *this;
  }
};

#endif
