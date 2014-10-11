#ifndef __DNA_H__
#define __DNA_H__

#include <vector>
#include "Color.h"
#include "Vec2.h"
#include "RandomGen.h"

enum InstructionType {
  INSTR_IDLE,
  INSTR_FORWARD,
  INSTR_ROTATE,
  INSTR_ABSORB,
  INSTR_EXCRETE,
  INSTR_METABOLIZE,
  INSTR_METABOLIZE2,
  INSTR_DIVIDE,
  INSTR_CMP_ENERGY,
  INSTR_RANDOM_BRANCH,
  NUM_INSTRS
};

const int DNA_SIZE = 32;

struct Instruction {
  InstructionType type;
  int next;

  union {
    struct {
      double speed;
    } forward;

    struct {
      double speed;
    } rotate;

    struct {
      int child_ip;
    } divide;

    struct {
      double threshold;
      int greater;
    } cmp_energy;

    struct {
      double probability;
      int branch;
    } random_branch;
  };

  static Instruction generate(RandomGen& g) {
    Instruction instr;
    instr.type = (InstructionType)g.int_range(0, NUM_INSTRS);
    instr.next = g.int_range(0, DNA_SIZE);

    switch (instr.type) {
      case INSTR_FORWARD:
        instr.forward.speed = g.range(0,4);
        break;
      case INSTR_ROTATE:
        instr.rotate.speed = g.range(0,10);
        break;
      case INSTR_DIVIDE:
        instr.divide.child_ip = g.int_range(0, DNA_SIZE);
        break;
      case INSTR_CMP_ENERGY:
        instr.cmp_energy.threshold = g.range(0,40);
        instr.cmp_energy.greater = g.int_range(0, DNA_SIZE);
        break;
      case INSTR_RANDOM_BRANCH:
        instr.random_branch.probability = g.range(0,1);
        instr.random_branch.branch = g.int_range(0, DNA_SIZE);
        break;
      default:
        break;
    }
    return instr;
  }

  void mutate(RandomGen& g) {
    if (g.range(0,1) < 0.1) {  // 10% chance of changing to new instruction
      *this = generate(g);
    }

    switch (type) {
      case INSTR_FORWARD:
        forward.speed += g.range(-1,1);  // XXX should use bell-style dist.
        break;
      case INSTR_ROTATE:
        rotate.speed += g.range(-1,1);
        break;
      case INSTR_DIVIDE:
        divide.child_ip = g.int_range(0, DNA_SIZE);
        break;
      case INSTR_CMP_ENERGY:
        cmp_energy.threshold += g.range(-5,5);
        if (g.range(0,1) < 0.1) {
          cmp_energy.greater = g.int_range(0, DNA_SIZE);
        }
        break;
      case INSTR_RANDOM_BRANCH:
        random_branch.probability += g.range(-0.3,0.3);
        random_branch.probability = clamp(0.0, 1.0, random_branch.probability);
        if (g.range(0,1) < 0.1) {
          random_branch.branch = g.int_range(0, NUM_INSTRS);
        }
        break;
      default:
        break;
    }
  }
};

class DNA {
 private:
  DNA() : code(DNA_SIZE), pigment(0,0,0) { }

 public:
  std::vector<Instruction> code;
  Color pigment;

  void mutate(RandomGen& g) {
    int instr = g.int_range(0, NUM_INSTRS);
    pigment.r = clamp(0.0, 1.0, pigment.r + g.range(-0.05,0.05));
    pigment.g = clamp(0.0, 1.0, pigment.g + g.range(-0.05,0.05));
    pigment.b = clamp(0.0, 1.0, pigment.b + g.range(-0.05,0.05));

    code[instr].mutate(g);
  }

  static DNA generate(RandomGen& g) {
    DNA dna;
    dna.pigment = Color(g.range(0,1), g.range(0,1), g.range(0,1));
    for (int i = 0; i < DNA_SIZE; i++) {
      dna.code[i] = Instruction::generate(g);
    }
    return dna;
  }
};

#endif
