#include <math.h>
#include <SDL3/SDL.h>
#include "app.h"
#include "sim.h"

double total_energy(AppState *app) {
  double acc = 0;
  loop_all(i) {
    Object me = app->universe[i];
    for (int j = i+1; j < app->n_objects; j++) {
      Object you = app->universe[j];
      Vec2 rij = sub(you.r, me.r);
      double r2 = dot(rij, rij);
      double invr = pow(r2, -0.5);
      double pref = app->gravity * me.m * you.m * invr;
      
      double vii = dot(me.v, me.v);
      double vjj = dot(you.v, you.v);
      double vij = dot(me.v, you.v);

      double rvi = dot(rij, me.v);
      double rvj = dot(rij, you.v);

      double corr = 3 * (vii + vjj) - 7 * vij - (rvi * rvj) / r2;
      corr /= 2 * app->c * app->c;
      corr -= 1;

      acc += pref * corr;  // GPE is negative
    }
    double v2 = dot(me.v, me.v);
    acc += 0.5 * me.m * v2; // KE is positive
    acc += 0.375 * me.m * v2*v2 / (app->c*app->c); // 1PN correction
  }
  return acc;
}

Object new_object(Vec2 r, Vec2 v, float m, float s) {
  return (Object) {
    r, v, {0,0},
    m, s, 
    {0, 0, 0, SDL_ALPHA_OPAQUE}
  };
}

Vec2 total_momentum(AppState *app) {
  Vec2 p = {0,0};
  loop_all(i) {
    Object obj = app->universe[i];
    p = add(p, scale(obj.v, obj.m));
  }
  return p;
}

double total_mass(AppState *app) {
  double m = 0;
  loop_all(i) {
    Object obj = app->universe[i];
    m += obj.m;
  }
  return m;
}

Vec2 centre_of_mass(AppState *app) {
  Vec2 com = {0,0};
  double m = 0;
  loop_all(i) {
    Object obj = app->universe[i];
    com = add(com, scale(obj.r, obj.m));
    m += obj.m;
  }
  return scale(com, 1/m);
}

double d2 (Vec2 a, Vec2 b) {
  double dx = (a.x-b.x);
  double dy = (a.y-b.y);
  return dx*dx + dy*dy;
}

