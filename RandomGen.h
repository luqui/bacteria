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

  RandomGen split() {
    return *this;
  }
};

#endif
