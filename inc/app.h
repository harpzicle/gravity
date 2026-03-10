#ifndef APP_H
#define APP_H

#define loop_all(var) for (int var = 0; var < app->n_objects; var++)

typedef struct {
  double x,y;
} Vec2;

typedef struct {
  Vec2 r, v, a;
  float m;
  float s;
  SDL_FColor c;
} Object;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;

  Object *universe;
  unsigned n_objects;
  unsigned capacity;
  double gravity;
  double c;
  double t;
  double dt;
  double scale;
  int running;

  int WIDTH;
  int HEIGHT;
} AppState;

double total_energy(AppState *app);
double total_mass(AppState *app);
Vec2 total_momentum(AppState *app);
Vec2 centre_of_mass(AppState *app);

double d2(Vec2 a, Vec2 b);

#endif