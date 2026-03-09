#include <SDL3/SDL.h>
#include <math.h>
#include "app.h"

double total_energy(AppState *app) {
  double acc = 0;
  loop_all(i) {
    Object me = app->universe[i];
    for (int j = i+1; j < app->n_objects; j++) {
      Object you = app->universe[j];
      double recip_d = pow(d2(me.r, you.r), -0.5);
      acc -= app->gravity * you.m * me.m * recip_d;  // GPE is negative
    }
    acc += d2((Vec2){0,0}, me.v) * me.m / 2; // KE is positive
  }
  return acc;
}

Vec2 total_momentum(AppState *app) {
  Vec2 acc = {0,0};
  loop_all(i) {
    Object obj = app->universe[i];
    acc.x += obj.v.x * obj.m;
    acc.y += obj.v.y * obj.m;
  }
  return acc;
}

double total_mass(AppState *app) {
  double acc = 0;
  loop_all(i) {
    Object obj = app->universe[i];
    acc += obj.m;
  }
  return acc;
}

Vec2 centre_of_mass(AppState *app) {
  Vec2 acc = {0,0};
  loop_all(i) {
    Object obj = app->universe[i];
    acc.x += obj.r.x * obj.m;
    acc.y += obj.r.y * obj.m;
  }
  double mass = total_mass(app);
  return (Vec2){acc.x / mass, acc.y / mass};
}

double d2 (Vec2 a, Vec2 b) {
  double dx = (a.x-b.x);
  double dy = (a.y-b.y);
  return dx*dx + dy*dy;
}