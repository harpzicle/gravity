#include <SDL3/SDL.h>
#include <math.h>
#include "app.h"
#include "sim.h"
#include "colour.h"

const float INFf = (float)1e999;
const double fudge = 1e-6;

void gr_corrections(AppState *app) {
  // Calculate approximate GR corrections
  Vec2 grc[app->n_objects];
  loop_all(i) {
    grc[i] = (Vec2) {0, 0};
  }

  loop_all(i) {
    for (int j = i+1; j < app->n_objects; j++) {
      Object me = app->universe[i], you = app->universe[j];
      Vec2 r = (Vec2) {me.r.x - you.r.x, me.r.y - you.r.y};
      Vec2 v = (Vec2) {me.v.x - you.v.x, me.v.y - you.v.y};
      double r2 = dot(r, r);
      double r1 = sqrt(r2);
      double r3 = r1*r2;

      double vi2 = dot(me.v, me.v);
      double vij = dot(me.v, you.v);
      double vj2 = dot(you.v, you.v);
      double riv = dot(r, me.v);
      double rjv = dot(r, you.v);
      double rai = dot(r, me.a);
      double raj = dot(r, you.a);

      double pref = app->gravity * you.m / (r3 * app->c * app->c);
      double rfactori = (
          4 * app->gravity * me.m / r1
        + 5 * app->gravity * you.m / r1 
        - vi2 
        - 2 * vj2 
        + 4 * vij
        + 1.5 * (rjv * rjv) / r2
        - 0.5 * raj
      );
      double rfactorj = (
          4 * app->gravity * you.m / r1
        + 5 * app->gravity * me.m / r1 
        - vj2 
        - 2 * vi2 
        + 4 * vij
        + 1.5 * (riv * riv) / r2
        - 0.5 * rai
      );

      Vec2 corri = scale(r, rfactori);
      Vec2 corrj = scale(r, rfactorj);

      double vfactori = (4 * riv - 3 * rjv);
      double vfactorj = (4 * rjv - 3 * riv);

      corri = add(corri, scale(v, vfactori));
      corrj = add(corrj, scale(v, vfactorj));

      corri = scale(corri, pref);
      corrj = scale(corrj, -pref);

      grc[i] = add(grc[i], corri);
      grc[j] = add(grc[j], corrj);
    }
  }

  // loop_all(i) {
  //   loop_all(j) {
  //     if (i == j) continue;
  //     Object me = app->universe[i], you = app->universe[j];
  //     Vec2 rij = (Vec2) {you.r.x - me.r.x, you.r.y - me.r.y};
  //     double r2 = dot(rij, rij) + fudge;
  //     double r = pow(r2, 0.5);
  //     double rij_factor = 4*app->gravity*me.m/r - dot(me.v, me.v) - dot(you.v, you.v);
  //     rij_factor += 4*dot(me.v, you.v) + dot(rij, you.a)/2;
  //     double rv = dot(rij, you.v); rv *= rv;
  //     rij_factor += 3*rv/(2*r2);

  //     Vec2 vij = (Vec2) {you.v.x - me.v.x, you.v.y - me.v.y};
  //     double vij_factor = dot(rij, (Vec2) {4*me.v.x - 3*you.v.x, 4*me.v.y - 3*you.v.y}) / r;

  //     Vec2 acc = (Vec2) {
  //       rij_factor * rij.x + vij_factor * vij.x,
  //       rij_factor * rij.y + vij_factor * vij.y
  //     };

  //     acc.x *= app->gravity * you.m / (r * r2 * app->c * app->c);
  //     acc.y *= app->gravity * you.m / (r * r2 * app->c * app->c);

  //     grc[i].x += acc.x;
  //     grc[i].y += acc.y;
  //   }
  // }

  loop_all(i) {
    app->universe[i].a.x += grc[i].x;
    app->universe[i].a.y += grc[i].y;
  }

}

void compute_accels(AppState *app) {
  loop_all(i) {
    app->universe[i].a = (Vec2) {0., 0.};
  }

  loop_all(i) {
    for (int j = i+1; j < app->n_objects; j++) {
      Object *me = &app->universe[i], *you = &app->universe[j];
      double invr3 = pow(d2(me->r, you->r) + fudge, -1.5);
      Vec2 force = scale(
        sub(you->r, me->r),
        app->gravity * invr3
      );
      me->a = add(me->a, scale(force, you->m));
      you->a = sub(you->a, scale(force, me->m));
    }
  }

  if (app->gr) gr_corrections(app);

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

const Colour_Luv slow = (Colour_Luv) { 29, 20, 8 };
const Colour_Luv fast = (Colour_Luv) { 80, -60, 30 };

void update_colours(AppState *app) {
  float maxvel = 0, minvel = INFf;
  loop_all(i) {
    Object me = app->universe[i];
    float speed = me.m * (dot(me.v, me.v));
    if (speed > maxvel) maxvel = speed;
    if (speed < minvel) minvel = speed;
  }
  loop_all(i) {
    Object *me = &app->universe[i];
    float speed = me->m * (dot(me->v, me->v));
    double t = unlerp(minvel, maxvel, speed);
    Colour_Luv Luv = (Colour_Luv) {
      lerp(slow.L, fast.L, t),
      lerp(slow.u, fast.u, t),
      lerp(slow.v, fast.v, t)
    };
    me->c = colour_luv2rgb(Luv);
  }
}

void sim_update_particles(AppState *app) {
  // euler_update(app);
  leapfrog_update(app);

  update_colours(app);

  app->t += app->dt;
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