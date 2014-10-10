#ifndef __COLOR_H__
#define __COLOR_H__

#include "SDL_opengl.h"

struct Color {
  double r, g, b;
  Color(double r, double g, double b) : r(r), g(g), b(b) { }

  void set() {
    glColor3d(r,g,b);
  }
};



#endif
