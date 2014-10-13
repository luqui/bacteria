#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <string>
#include <vector>
#include <stack>
#include "SDL_opengl.h"
#include "Vec2.h"
#include "Color.h"
#include "RandomGen.h"

enum Resource { RES_NONE, RES_ENERGY, RES_POOP, RES_DESSERT };

inline std::string show_resource(Resource r) {
  switch(r) {
    case RES_NONE: return "NONE";
    case RES_ENERGY: return "ENERGY";
    case RES_POOP: return "POOP";
    case RES_DESSERT: return "DESSERT";
    default: return "???";
  }
}

class ResourceField {
 private:
  ResourceField(const ResourceField&); // no copying

  Vec2 ll, ur;
  int dim_x, dim_y;
  double regen;

  std::vector< std::vector< std::stack<Resource> > > grid;

  bool to_grid(Vec2 position, int* x, int* y) {
    *x = int(round(dim_x * ((position.x - ll.x) / (ur.x - ll.x))));
    *y = int(round(dim_y * ((position.y - ll.y) / (ur.y - ll.y))));
    return 0 <= *x && *x < dim_x && 0 <= *y && *y < dim_y;
  }

 public:
  ResourceField(Vec2 ll, Vec2 ur, int dim_x, int dim_y)
      : ll(ll), ur(ur), dim_x(dim_x), dim_y(dim_y), regen(0),
        grid(dim_x, std::vector< std::stack<Resource> >(dim_y))
  {
    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        grid[i][j].push(RES_ENERGY);
      }
    }
  }

  static Color resource_color(Resource res) {
    switch (res) {
      case RES_NONE: return Color(0,0,0);
      case RES_ENERGY: return Color(0.2,0.2,0);
      case RES_POOP: return Color(0.2,0.1,0);
      case RES_DESSERT: return Color(0.2,0,0.2);
      default: abort();
    }
  }

  void draw() {
    double dx = (ur.x - ll.x) / dim_x;
    double dy = (ur.y - ll.y) / dim_y;

    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        Vec2 p = ll + Vec2(i * dx, j * dy);
        if (!grid[i][j].empty()) {
          double scale = 2-std::exp(-0.1*grid[i][j].size());
          (scale*resource_color(grid[i][j].top())).set();
          glBegin(GL_POLYGON);
            glVertex2d(p.x - dx/2, p.y - dy/2);
            glVertex2d(p.x - dx/2, p.y + dy/2);
            glVertex2d(p.x + dx/2, p.y + dy/2);
            glVertex2d(p.x + dx/2, p.y - dy/2);
          glEnd();
        }
      }
    }
  }

  void step(double dt, RandomGen& gen) {
    regen -= dt;
    while (regen < 0) {
      int i = std::min((int)gen.range(0, dim_x), dim_x);
      int j = std::min((int)gen.range(0, dim_y), dim_y);
      grid[i][j].push(RES_ENERGY);

      // poisson
      // XXX possible infinite loop if gen.range(0,1) can return exactly 1
      regen += -std::log(1 - gen.range(0,1)) * 0.2;
    }
  }

  Resource take(Vec2 position) {
    int x, y;
    if (to_grid(position, &x, &y)) {
      if (grid[x][y].empty()) {
        return RES_NONE;
      }
      else {
        Resource r = grid[x][y].top();
        grid[x][y].pop();
        return r;
      }
    }
    else {
      abort();
    }
  }

  void put(Vec2 position, Resource r) {
    int x, y;
    if (to_grid(position, &x, &y)) {
      grid[x][y].push(r);
    }
    else {
      abort();
    }
  }
};

#endif
