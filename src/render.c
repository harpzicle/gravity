#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include <SDL3/SDL.h>

#include "app.h"

extern char *text_buffer;
extern const int n_text_buffer;

Vec2 render_to_pos(AppState *app, Vec2 render) {
  Vec2 pos = {
    (render.x - app->WIDTH/2) / app->scale,
    (render.y - app->HEIGHT/2) / app->scale
  };
  return pos;
}

Vec2 pos_to_render(AppState *app, Vec2 pos) {
  Vec2 render = {
    pos.x * app->scale + app->WIDTH / 2,
    pos.y * app->scale + app->HEIGHT / 2
  };
  return render;
}

SDL_FRect render_position(AppState *app, Object obj) {
  SDL_FRect ans;
  ans.x = obj.r.x * app->scale + app->WIDTH / 2;
  ans.y = obj.r.y * app->scale + app->HEIGHT / 2;

  ans.w = obj.s;
  ans.h = obj.s;

  return ans;
}

void render_objects(AppState *app) {
  SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  SDL_FRect rects[app->n_objects] = {};

  loop_all(i) {
    rects[i] = render_position(app, app->universe[i]);
  }
  
  SDL_RenderFillRects(app->renderer, rects, app->n_objects);
}

void render_linef(AppState *app, int is_right_aligned, int line, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int length = vsnprintf(text_buffer, n_text_buffer, fmt, args);
  float x = is_right_aligned ? app->WIDTH-1 - 8*length : 1;
  float y = line * 8 + 1;

  SDL_RenderDebugText(app->renderer, x, y, text_buffer);
  va_end(args);
}

void render_diagnostics(AppState *app) {
  SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
  
  Vec2 com = centre_of_mass(app);
  SDL_RenderPoint(
    app->renderer,
    com.x * app->scale + app->WIDTH / 2,
    com.y * app->scale + app->HEIGHT / 2
  );
  
  
  SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  render_linef(app, 0, 0, "[/] Timestep: %.2g", app->dt);
  render_linef(app, 0, 1, "P Paused: %s", "Yes\0No" + 4*app->running);
  render_linef(app, 0, 2, "S Step");
  render_linef(app, 0, 3, "R Reverse");
  render_linef(app, 0, 4, "C Recentre");

  render_linef(app, 1, 0, "Time: %12.10lf", app->t);
  render_linef(app, 1, 1, "Momentum: %#8.4g", sqrt(d2((Vec2){0,0},total_momentum(app))));
  render_linef(app, 1, 2, "Energy: %#8.4g", total_energy(app));
}

void render_frame(AppState *app) {
  SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(app->renderer);
  
  render_objects(app);
  render_diagnostics(app);

  SDL_RenderPresent(app->renderer);
}
