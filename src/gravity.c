#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <math.h>

#include "app.h"
#include "sim.h"
#include "render.h"
#include "event.h"
#include "colour.h"

const char* title = "Gravity Simulation";
const char* version = "0.3";

char *text_buffer;
const int n_text_buffer = 140;

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
  app->c = 1000;
  app->t = 0;
  app->dt= 0.001;
  app->scale = 100;
  app->running = 1;
  app->gr = 1;

  app->universe[0] = new_object(
    (Vec2){0., 0.}, 
    (Vec2){0., 0.},
    1.7,
    10.
  );
  app->universe[1] = new_object(
    (Vec2){2., 0.},
    (Vec2){0., 1.1},
    0.3,
    5.
  );
  app->universe[2] = new_object(
    (Vec2){0., -2.},
    (Vec2){-1.6, -0.2},
    0.2,
    3.
  );
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

  switch (event->type) {

  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;

  case SDL_EVENT_KEY_DOWN:
    return event_keydown(app, event->key);

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    return event_mousedown(app, event->button);

  case SDL_EVENT_MOUSE_BUTTON_UP:
    return event_mouseup(app, event->button);

  case SDL_EVENT_MOUSE_WHEEL:
    return event_mousewheel(app, event->wheel);

  default:
    return SDL_APP_CONTINUE;
  }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *app = appstate;

  if (app->running) sim_update_particles(app);

  render_frame(app);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState *app = appstate;

  SDL_free(app->universe);
  SDL_free(text_buffer);
  SDL_free(app);
}