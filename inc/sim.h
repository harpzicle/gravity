#ifndef SIM_H
#define SIM_H

void sim_update_particles(AppState *app);
void sim_recenter(AppState *app);
void sim_update(AppState *app);

inline double dot(Vec2 a, Vec2 b) {
  return a.x * b.x + a.y * b.y;
}

inline Vec2 add(Vec2 a, Vec2 b) {
  return (Vec2) {a.x + b.x, a.y + b.y};
}

inline Vec2 sub(Vec2 a, Vec2 b) {
  return (Vec2) {a.x - b.x, a.y - b.y};
}

inline Vec2 scale(Vec2 v, double s) {
  return (Vec2) {v.x * s, v.y * s};
}

inline double lerp(double a, double b, double t) {
  return t*b + (1-t)*a;
}

inline double unlerp(double a, double b, double c) {
  return (c-a) / (b-a);
}

#endif