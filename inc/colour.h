#ifndef COLOUR_H
#define COLOUR_H

typedef struct {
  float L, u, v;
} Colour_Luv;

SDL_FColor colour_luv2rgb(Colour_Luv Luv);

#endif