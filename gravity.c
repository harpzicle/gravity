#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const char* title = "Gravity Simulation";
const char* version = "0.2";

const int WIDTH = 1080;
const int HEIGHT = 720;

static SDL_Window *window;
static SDL_Renderer *renderer;

typedef struct {
  double x,y;
} Vec2;

typedef struct {
  Vec2 r, v, a;
  float m;
  float s;
  SDL_FColor c;
} Object;

Object *universe;
unsigned n_objects;
unsigned capacity;
const double gravity = 2.5;
const double scale = 100;
double dt = 0.001;
double t = 0;
int running = 1;

char *text_buffer; const int n_text_buffer = 1024;

double d2(Vec2 a, Vec2 b) {
  double p = (a.x-b.x) / scale;
  double q = (a.y-b.y) / scale;
  return p*p + q*q;
}

Vec2 *accel;

void compute_accels() {
  for (int i = 0; i < n_objects; i++) {
    universe[i].a = (Vec2) {0., 0.};
  }

  for (int i = 0; i < n_objects; i++) {
    for (int j = i+1; j < n_objects; j++) {
      Object *me = &universe[i], *you = &universe[j];
      double recip_d = pow(d2(me->r, you->r) + 1e-7, -1.5);
      Vec2 force = {
        gravity * (you->r.x - me->r.x) * recip_d / scale,
        gravity * (you->r.y - me->r.y) * recip_d / scale
      };
      me->a.x += force.x * you->m; me->a.y += force.y * you->m;
      you->a.x -= force.x * me->m; you->a.y -= force.y * me->m;
    }
  }
}

double total_energy() {
  double acc = 0;
  for (int i = 0; i < n_objects; i++) {
    Object me = universe[i];
    for (int j = i+1; j < n_objects; j++) {
      Object you = universe[j];
      double recip_d = pow(d2(me.r, you.r), -0.5);
      acc -= gravity * you.m * me.m * recip_d / scale;
    }
    acc += d2((Vec2){0,0}, me.v) * me.m * scale / 2;
  }
  return acc;
}

Vec2 total_momentum() {
  Vec2 acc = {0,0};
  for (int i = 0; i < n_objects; i++) {
    Object obj = universe[i];
    acc.x += obj.v.x * obj.m;
    acc.y += obj.v.y * obj.m;
  }
  return acc;
}

double total_mass() {
  double acc = 0;
  for (int i = 0; i < n_objects; i++) {
    Object obj = universe[i];
    acc += obj.m;
  }
  return acc;
}

Vec2 centre_of_mass() {
  Vec2 acc = {0,0};
  for (int i = 0; i < n_objects; i++) {
    Object obj = universe[i];
    acc.x += obj.r.x * obj.m;
    acc.y += obj.r.y * obj.m;
  }
  double mass = total_mass();
  return (Vec2){acc.x / mass, acc.y / mass};
}


void euler_update() {
  compute_accels();
  for (int i = 0; i < n_objects; i++) {
    Object *me = &universe[i];
    me->v.x += me->a.x * dt;
    me->v.y += me->a.y * dt;

    me->r.x += me->v.x * dt * scale;
    me->r.y += me->v.y * dt * scale;
  }
}
void leapfrog_update() {
  compute_accels();
  for (int i = 0; i < n_objects; i++) {
    Object *me = &universe[i];
    me->v.x += me->a.x * dt / 2;
    me->v.y += me->a.y * dt / 2;

    me->r.x += me->v.x * dt * scale;
    me->r.y += me->v.y * dt * scale;
  }
  compute_accels();
  for(int i = 0; i < n_objects; i++) {
    Object *me = &universe[i];
    me->v.x += me->a.x * dt / 2;
    me->v.y += me->a.y * dt / 2;
  }
}

void update_particles() {
  // euler_update();
  leapfrog_update();
}

void recenter() {
  double mass = total_mass();
  Vec2 com_v = total_momentum();
  com_v.x /= mass;
  com_v.y /= mass;
  Vec2 com_r = centre_of_mass();
  for (int i = 0; i < n_objects; i++) {
    Object *me = &universe[i];
    me->r.x -= com_r.x - WIDTH/2;
    me->r.y -= com_r.y - HEIGHT/2;
    me->v.x -= com_v.x;
    me->v.y -= com_v.y;
  } 
}

float frand0(float max) {
  return ((float)rand() / RAND_MAX) * max;
}
float frand(float min, float max) {
  return frand0(max - min) + min;
}
float frand_margin(float max, float margin) {
  return frand(margin, max-margin);
}

SDL_AppResult SDL_AppInit (void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata(title, version, NULL);
  
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    SDL_Log("Failed to initialise SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  
  if (!SDL_CreateWindowAndRenderer(title, WIDTH, HEIGHT, 0, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  
  srand(13357);

  text_buffer = malloc(n_text_buffer);
  n_objects = 3;
  capacity = 2 * n_objects;
  universe = calloc(sizeof *universe, capacity);

  universe[0] = (Object){
    {540., 360.}, 
    {0., 0.},
    {0., 0.},
    1.7,
    10.,
    {255, 255, 255, SDL_ALPHA_OPAQUE}
  };
  universe[1] = (Object){
    {740., 360.},
    {0., 1.1},
    {0., 0.},
    0.3,
    5.,
    {255, 255, 255, SDL_ALPHA_OPAQUE}
  };
  universe[2] = (Object){
    {540., 160.},
    {-1.6, -0.2},
    {0., 0.},
    0.2,
    3.,
    {255, 255, 255, SDL_ALPHA_OPAQUE}
  };
  // for (int i = 3; i < n_objects; i++) {
  //   universe[i] = (Object){
  //     {frand(100,300),frand(320,400)},
  //     {frand(-0.2, 0.2), frand(0.7, 1.3)},
  //     0.01, 1
  //   };
  // }

  recenter();

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  if (event->type == SDL_EVENT_KEY_DOWN) {
    switch (event->key.key) {
    
    case SDLK_ESCAPE:
      return SDL_APP_SUCCESS;

    case SDLK_LEFTBRACKET:
      dt /= 2;
      break;
    case SDLK_RIGHTBRACKET:
      dt *= 2;
      break;
    
    case SDLK_P:
      running = !running;
      break;

    case SDLK_R:
      dt *= -1;
      break;
    
    case SDLK_S:
      update_particles();
      break;

    case SDLK_C:
      recenter();
      break;

    default:
      break;
    }
  }
  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
    running = 0;

    if (n_objects == capacity) {
      Object *temp = universe;
      unsigned new_cap = capacity * 3 / 2;
      universe = realloc(universe, new_cap * sizeof *universe);
      if (universe == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "failed allocating space for new planet");
        universe = temp;
        return SDL_APP_FAILURE;
      }

      capacity = new_cap;
    }

    universe[n_objects].r = (Vec2) { event->button.x, event->button.y };
  }
  if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
    double vx, vy;
    vx = (event->button.x - universe[n_objects].r.x) / scale;
    vy = (event->button.y - universe[n_objects].r.y) / scale;
    universe[n_objects].v = (Vec2) {vx, vy};
    universe[n_objects].a = (Vec2) {0, 0};
    universe[n_objects].m = 0.1;
    universe[n_objects].s = 3.;
    n_objects++;
  }
  
  return SDL_APP_CONTINUE;
}

void render_objects(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  SDL_FRect rects[n_objects] = {};

  for (int i = 0; i < n_objects; i++) {
    rects[i] = render_position(universe[i]);
  }
  
  SDL_RenderFillRects(renderer, rects, n_objects);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
  
  if (running) {
    update_particles();
    t += dt;
  }

  render_objects(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
  Vec2 com = centre_of_mass();
  SDL_RenderPoint(renderer, com.x, com.y);
  
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  int length;
  snprintf(text_buffer, n_text_buffer, "[/] Timestep: %.2g", dt);
  SDL_RenderDebugText(renderer, 1, 1, text_buffer);
  snprintf(text_buffer, n_text_buffer, "P Paused: %s", "Yes\0No" + 4*!!running);
  SDL_RenderDebugText(renderer, 1, 10, text_buffer);
  snprintf(text_buffer, n_text_buffer, "S Step");
  SDL_RenderDebugText(renderer, 1, 19, text_buffer);
  snprintf(text_buffer, n_text_buffer, "R Reverse");
  SDL_RenderDebugText(renderer, 1, 28, text_buffer);
  snprintf(text_buffer, n_text_buffer, "C Recentre");
  SDL_RenderDebugText(renderer, 1, 37, text_buffer);

  length = snprintf(text_buffer, n_text_buffer, "Time: %12.10lf", t);
  SDL_RenderDebugText(renderer, WIDTH-1 - 8*length, 1, text_buffer);
  length = snprintf(text_buffer, n_text_buffer, "Momentum: %#8.4g", sqrt(d2((Vec2){0,0},total_momentum())));
  SDL_RenderDebugText(renderer, WIDTH-1 - 8*length, 10, text_buffer);
  length = snprintf(text_buffer, n_text_buffer, "Energy: %#8.4g", total_energy());
  SDL_RenderDebugText(renderer, WIDTH-1 - 8*length, 19, text_buffer);

  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  free(universe);
}