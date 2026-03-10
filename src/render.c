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

int in_rect(Vec2 pos, Vec2 rect) {
  return (pos.x < rect.x && pos.x > 0 && pos.y < rect.x && pos.y > 0);
}

Vec2 frame_clip(Vec2 pos, float s, float w, float h) {
  s /= 2;
  // project a particle outside the screen onto the edge
  if (pos.x > s && pos.y > s && pos.x < w-s &&  pos.y < h-s)
    return pos;

  double t = (s - h/2) / (pos.y - h/2);
  Vec2 bottom = (Vec2) {w/2 - (pos.x-w/2)*t, h-s};
  Vec2 top = (Vec2) {w/2 + (pos.x-w/2)*t, s};

  t = (s - w/2) / (pos.x - w/2);
  Vec2 left = (Vec2) {s, h/2 + (pos.y-h/2)*t};
  Vec2 right = (Vec2) {w-s, h/2 - (pos.y-h/2)*t};

  if (pos.x <= s) {
    return (left.y < s) ? top : (left.y > h-s) ? bottom : left;
  }
  if (pos.x >= w-s) {
    return (right.y < s) ? top : (right.y > h-s) ? bottom : right;
  }
  if (pos.y <= s) {
    return (top.x < s) ? left : (top.x > w-s) ? right : top;
  }
  if (pos.y >= h-s) {
    return (bottom.x < s) ? left : (bottom.x > w-s) ? right : bottom;
  }
}

SDL_FRect render_position(AppState *app, Object obj) {
  Vec2 pos = frame_clip(pos_to_render(app, obj.r), obj.s, app->WIDTH, app->HEIGHT);
  SDL_FRect ans;
  ans.x = pos.x;
  ans.y = pos.y;
  ans.w = obj.s;
  ans.h = obj.s;

  return ans;
}


void render_objects(AppState *app) {
  SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  SDL_Vertex vertices[6 * app->n_objects] = {};

  loop_all(i) {
    Object me = app->universe[i];
    SDL_FRect pos = render_position(app, me);
    vertices[6*i+0] = (SDL_Vertex) {{pos.x-me.s/2, pos.y-me.s/2}, me.c, {0, 0}}; // top left
    vertices[6*i+1] = (SDL_Vertex) {{pos.x-me.s/2, pos.y+me.s/2}, me.c, {0, 0}}; // bottom left
    vertices[6*i+2] = (SDL_Vertex) {{pos.x+me.s/2, pos.y-me.s/2}, me.c, {0, 0}}; // top right
    
    vertices[6*i+3] = (SDL_Vertex) {{pos.x-me.s/2, pos.y+me.s/2}, me.c, {0, 0}}; // bottom left
    vertices[6*i+4] = (SDL_Vertex) {{pos.x+me.s/2, pos.y+me.s/2}, me.c, {0, 0}}; // bottom right
    vertices[6*i+5] = (SDL_Vertex) {{pos.x+me.s/2, pos.y-me.s/2}, me.c, {0, 0}}; // top right
  }
  
  SDL_RenderGeometry(app->renderer, NULL, vertices, 6*app->n_objects, NULL, 0);
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
  render_linef(app, 0, 2, "Scale: %.1f", app->scale);
  render_linef(app, 0, 3, "S Step");
  render_linef(app, 0, 4, "R Reverse");
  render_linef(app, 0, 5, "C Recentre");

  render_linef(app, 1, 0, "Time: %12.10lf", app->t);
  render_linef(app, 1, 1, "Momentum: %#8.4g", sqrt(d2((Vec2){0,0},total_momentum(app))));
  render_linef(app, 1, 2, "Energy: %#8.4f", total_energy(app));
}

void render_frame(AppState *app) {
  SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(app->renderer, 26, 26, 26, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(app->renderer);
  
  render_objects(app);
  render_diagnostics(app);

  SDL_RenderPresent(app->renderer);
}
