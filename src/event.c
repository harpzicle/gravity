#include <SDL3/SDL.h>
#include "app.h"
#include "sim.h"
#include "render.h"

SDL_AppResult event_keydown(AppState *app, SDL_KeyboardEvent event) {
  switch (event.key) {
    
  case SDLK_ESCAPE:
    return SDL_APP_SUCCESS;

  case SDLK_LEFTBRACKET:
    app->dt /= 2;
    break;
  case SDLK_RIGHTBRACKET:
    app->dt *= 2;
    break;
  
  case SDLK_P:
    app->running = !app->running;
    break;

  case SDLK_R:
    app->dt *= -1;
    break;
  
  case SDLK_S:
    sim_update_particles(app);
    break;

  case SDLK_C:
    sim_recenter(app);
    break;

  default:
    break;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult event_mousedown(AppState *app, SDL_MouseButtonEvent event) {
  app->running = 0;

  if (app->n_objects == app->capacity) {
    Object *temp = app->universe;
    unsigned new_cap = app->capacity * 3 / 2;

    app->universe = SDL_realloc(app->universe, new_cap * sizeof *app->universe);
    if (app->universe == NULL) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "failed allocating space for new planet");
      app->universe = temp;
      return SDL_APP_FAILURE;
    }

    app->capacity = new_cap;
  }

  Object *new_obj = &app->universe[app->n_objects];

  new_obj->r = render_to_pos(app, (Vec2) { event.x, event.y });
  new_obj->a = (Vec2) {0, 0};
  new_obj->m = 0.1;
  new_obj->s = 3.;
  new_obj->c = (SDL_FColor){255, 255, 0, SDL_ALPHA_OPAQUE}; 

  return SDL_APP_CONTINUE;
}

SDL_AppResult event_mouseup(AppState *app, SDL_MouseButtonEvent event) {
  double vx, vy;
  Vec2 click = pos_to_render(app, app->universe[app->n_objects].r);
  vx = (event.x - click.x) / app->scale;
  vy = (event.y - click.y) / app->scale;
  app->universe[app->n_objects].v = (Vec2) {vx, vy};
  app->n_objects++;

  return SDL_APP_CONTINUE;
}

SDL_AppResult event_mousewheel(AppState *app, SDL_MouseWheelEvent event) {
  int up = event.y * ( event.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1 );
  if (up > 0) {
    app->scale *= 1.5;
  } else if (up < 0) {
    app->scale /= 1.5;
  }

  return SDL_APP_CONTINUE;
}