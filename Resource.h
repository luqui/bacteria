#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include "SDL_opengl.h"
#include "Vec2.h"
#include "Color.h"
#include "RandomGen.h"

typedef int Resource;

const Resource RES_NONE = 0;
const Resource RES_MAX = 8;

inline std::string show_resource(Resource r) {
  std::stringstream ss;
  ss << r;
  return ss.str();
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
        grid[i][j].push(1);
      }
    }
  }

  static Color resource_color(Resource res) {
    int green = res % 3;
    int rb = res / 3;
    return Color(rb / 6.0, green / 6.0, rb / 6.0);
  }

  void draw() {
    double dx = (ur.x - ll.x) / dim_x;
    double dy = (ur.y - ll.y) / dim_y;

    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        Vec2 p = ll + Vec2(i * dx, j * dy);
        if (!grid[i][j].empty()) {
          resource_color(grid[i][j].top()).set();
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
    /*
    regen -= dt;
    while (regen < 0) {
      int i = std::min((int)gen.range(0, dim_x), dim_x);
      int j = std::min((int)gen.range(0, dim_y), dim_y);
      if (grid[i][j].size() > 1 && grid[i][j].top() > 1) {
        grid[i][j].top()--;
      }

      // poisson
      // XXX possible infinite loop if gen.range(0,1) can return exactly 1
      regen += -std::log(1 - gen.range(0,1)) * 0.2;
    }
    */
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
