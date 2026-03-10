#include <SDL3/SDL.h>
#include "app.h"
#include <math.h>

double dot(Vec2 a, Vec2 b) {
  return a.x * b.x + a.y * b.y;
}

static const double fudge = 1e-6;

void compute_accels(AppState *app) {
  loop_all(i) {
    app->universe[i].a = (Vec2) {0., 0.};
  }

  loop_all(i) {
    for (int j = i+1; j < app->n_objects; j++) {
      Object *me = &app->universe[i], *you = &app->universe[j];
      double recip_d = pow(d2(me->r, you->r) + fudge, -1.5);
      Vec2 force = {
        app->gravity * (you->r.x - me->r.x) * recip_d,
        app->gravity * (you->r.y - me->r.y) * recip_d
      };
      me->a.x += force.x * you->m; me->a.y += force.y * you->m;
      you->a.x -= force.x * me->m; you->a.y -= force.y * me->m;
    }
  }

  // Calculate approximate GR corrections
  Vec2 grc[app->n_objects];
  loop_all(i) {
    grc[i] = (Vec2) {0, 0};
  }
  loop_all(i) {
    loop_all(j) {
      if (i == j) continue;
      Object me = app->universe[i], you = app->universe[j];
      Vec2 rij = (Vec2) {you.r.x - me.r.x, you.r.y - me.r.y};
      double r2 = dot(rij, rij) + fudge;
      double r = pow(r2, 0.5);
      double rij_factor = 4*app->gravity*me.m/r - dot(me.v, me.v) - dot(you.v, you.v);
      rij_factor += 4*dot(me.v, you.v) + dot(rij, you.a)/2;
      double rv = dot(rij, you.v); rv *= rv;
      rij_factor += 3*rv/(2*r2);

      Vec2 vij = (Vec2) {you.v.x - me.v.x, you.v.y - me.v.y};
      double vij_factor = dot(rij, (Vec2) {4*me.v.x - 3*you.v.x, 4*me.v.y - 3*you.v.y}) / r;

      Vec2 acc = (Vec2) {
        rij_factor * rij.x + vij_factor * vij.x,
        rij_factor * rij.y + vij_factor * vij.y
      };

      acc.x *= app->gravity * you.m / (r * r2 * app->c * app->c);
      acc.y *= app->gravity * you.m / (r * r2 * app->c * app->c);

      grc[i].x += acc.x;
      grc[i].y += acc.y;
    }
  }

  loop_all(i) {
    app->universe[i].a.x += grc[i].x;
    app->universe[i].a.y += grc[i].y;
  }

}


void euler_update(AppState *app) {
  compute_accels(app);
  loop_all(i) {
    Object *me = &app->universe[i];
    me->v.x += me->a.x * app->dt;
    me->v.y += me->a.y * app->dt;

    me->r.x += me->v.x * app->dt;
    me->r.y += me->v.y * app->dt;
  }
}
void leapfrog_update(AppState *app) {
  compute_accels(app);
  loop_all(i) {
    Object *me = &app->universe[i];
    me->v.x += me->a.x * app->dt / 2;
    me->v.y += me->a.y * app->dt / 2;

    me->r.x += me->v.x * app->dt;
    me->r.y += me->v.y * app->dt;
  }
  compute_accels(app);
  for(int i = 0; i < app->n_objects; i++) {
    Object *me = &app->universe[i];
    me->v.x += me->a.x * app->dt / 2;
    me->v.y += me->a.y * app->dt / 2;
  }
}

void sim_update_particles(AppState *app) {
  // euler_update(app);
  leapfrog_update(app);
}

void sim_recenter(AppState *app) {
  double mass = total_mass(app);
  Vec2 com_v = total_momentum(app);
  com_v.x /= mass;
  com_v.y /= mass;
  Vec2 com_r = centre_of_mass(app);
  loop_all(i) {
    Object *me = &app->universe[i];
    me->r.x -= com_r.x;
    me->r.y -= com_r.y;
    me->v.x -= com_v.x;
    me->v.y -= com_v.y;
  } 
}

void sim_update(AppState *app) {
  if (app->running) {
    sim_update_particles(app);
    app->t += app->dt;
  }
}