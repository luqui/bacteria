#ifndef __VEC2_H__
#define __VEC2_H__

struct Vec2 {
  Vec2(double x, double y) : x(x), y(y) { }
  double x, y;

  Vec2& operator *= (double a) {
    x *= a;
    y *= a;
    return *this;
  }

  Vec2& operator += (const Vec2& v) {
    this->x += v.x;
    this->y += v.y;
    return *this;
  }

  Vec2& operator -= (const Vec2& v) {
    this->x -= v.x;
    this->y -= v.y;
    return *this;
  }
};

static Vec2 operator * (double a, Vec2 v) {
  return v *= a;
}

static Vec2 operator + (Vec2 u, const Vec2& v) {
  return u += v;
}

static Vec2 operator - (Vec2 u, const Vec2& v) {
  return u -= v;
}

#endif
