#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <math.h>

#include "app.h"
#include "sim.h"
#include "render.h"

const char* title = "Gravity Simulation";
const char* version = "0.2";

char *text_buffer;
const int n_text_buffer = 1024;

SDL_AppResult SDL_AppInit (void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata(title, version, NULL);
  AppState *app = SDL_malloc(sizeof *app);
  *appstate = app;

  app->WIDTH = 1080;
  app->HEIGHT = 720;
  
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    SDL_Log("Failed to initialise SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  
  if (!SDL_CreateWindowAndRenderer(title, app->WIDTH, app->HEIGHT, 0, &app->window, &app->renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  
  text_buffer = SDL_malloc(n_text_buffer);
  app->n_objects = 3;
  app->capacity = 2 * app->n_objects;
  app->universe = SDL_calloc(sizeof *app->universe, app->capacity);
  app->gravity = 2.5;
  app->t = 0;
  app->dt= 0.001;
  app->scale = 100;
  app->running = 1;

  app->universe[0] = (Object){
    {0., 0.}, 
    {0., 0.},
    {0., 0.},
    1.7,
    10.,
    {255, 255, 0, SDL_ALPHA_OPAQUE}
  };
  app->universe[1] = (Object){
    {2., 0.},
    {0., 1.1},
    {0., 0.},
    0.3,
    5.,
    {255, 0, 255, SDL_ALPHA_OPAQUE}
  };
  app->universe[2] = (Object){
    {0., -2.},
    {-1.6, -0.2},
    {0., 0.},
    0.2,
    3.,
    {0, 255, 255, SDL_ALPHA_OPAQUE}
  };
  // for (int i = 3; i < n_objects; i++) {
  //   universe[i] = (Object){
  //     {frand(100,300),frand(320,400)},
  //     {frand(-0.2, 0.2), frand(0.7, 1.3)},
  //     0.01, 1
  //   };
  // }

  sim_recenter(app);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState *app = appstate;

  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  if (event->type == SDL_EVENT_KEY_DOWN) {
    switch (event->key.key) {
    
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
  }
  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
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

    new_obj->r = render_to_pos(app, (Vec2) { event->button.x, event->button.y });
    new_obj->a = (Vec2) {0, 0};
    new_obj->m = 0.1;
    new_obj->s = 3.;
    new_obj->c = (SDL_FColor){255, 255, 0, SDL_ALPHA_OPAQUE}; 

  }
  if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
    double vx, vy;
    Vec2 click = pos_to_render(app, app->universe[app->n_objects].r);
    vx = (event->button.x - click.x) / app->scale;
    vy = (event->button.y - click.y) / app->scale;
    app->universe[app->n_objects].v = (Vec2) {vx, vy};
    app->n_objects++;
  }

  if (event->type == SDL_EVENT_MOUSE_WHEEL) {
    int up = event->wheel.y * ( event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1 );
    if (up > 0) {
      app->scale *= 1.5;
    } else if (up < 0) {
      app->scale /= 1.5;
    }
  }
  
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *app = appstate;

  sim_update(app);

  render_frame(app);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState *app = appstate;
  SDL_free(app->universe);
  SDL_free(text_buffer);
  SDL_free(app);
}