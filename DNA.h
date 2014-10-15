#ifndef __DNA_H__
#define __DNA_H__

#include <iostream>
#include <vector>
#include "Color.h"
#include "Vec2.h"
#include "RandomGen.h"

enum InstructionType {
  INSTR_IDLE,
  INSTR_FORWARD,
  INSTR_ROTATE,
  INSTR_METABOLIZE,
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
      int depth;
    } metabolize;

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
      case INSTR_METABOLIZE:
        instr.metabolize.depth = g.int_range(1,4);
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
      case INSTR_METABOLIZE:
        metabolize.depth += g.int_range(-1,2);
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

  void dump(std::ostream& out) {
    switch (type) {
      case INSTR_IDLE:
        out << "IDLE";
        break;
      case INSTR_FORWARD:
        out << "FORWARD(" << forward.speed << ")";
        break;
      case INSTR_ROTATE:
        out << "ROTATE(" << rotate.speed << ")";
        break;
      case INSTR_METABOLIZE:
        out << "METABOLIZE(" << metabolize.depth << ")";
        break;
      case INSTR_DIVIDE:
        out << "DIVIDE";
        break;
      case INSTR_CMP_ENERGY:
        out << "CMP_ENERGY(> " << cmp_energy.threshold << " ? " << cmp_energy.greater << ")";
        break;
      case INSTR_RANDOM_BRANCH:
        out << "CMP_RANDOM_BRANCH(p=" << random_branch.probability << " ? "
                                      << random_branch.branch << ")";
        break;
      default:
        out << "???";
        break;
    }
    out << " --> " << next;
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
    pigment.r = clamp(0.0, 1.0, pigment.r + g.range(-0.01,0.01));
    pigment.g = clamp(0.0, 1.0, pigment.g + g.range(-0.01,0.01));
    pigment.b = clamp(0.0, 1.0, pigment.b + g.range(-0.01,0.01));

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

  void dump(std::ostream& out) {
    for (int i = 0; i < DNA_SIZE; i++) {
      out << i << ": ";
      code[i].dump(out);
      out << "\n";
    }
  }
};

#endif
