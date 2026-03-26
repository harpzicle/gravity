#include <SDL3/SDL.h>
#include <math.h>
#include "colour.h"

typedef struct {
  float X,Y,Z;
} XYZ;
typedef struct {
  float r,g,b;
} RGB;

XYZ luv2xyz (Colour_Luv Luv) {
  // http://www.easyrgb.com/index.php?X=MATH&H=17
  float var_Y = (Luv.L + 16) / 116;
  float temp = var_Y * var_Y * var_Y;
  if (temp > 0.008856) var_Y = temp;
  else                 var_Y = (var_Y - 16/116) / 7.787;

  const float ref_X = 95.047;
  const float ref_Y = 100.000;
  const float ref_Z = 108.883;

  temp = ref_X + (15*ref_Y) + (3*ref_Z);
  const float ref_U = 4*ref_X / temp;
  const float ref_V = 9*ref_Y / temp;

  const float var_U = Luv.u / (13*Luv.L) + ref_U;
  const float var_V = Luv.v / (13*Luv.L) + ref_V;

  const float Y = var_Y * 100;
  const float X = -(9*Y*var_U) / ((var_U-4)*var_V - var_U*var_V);
  const float Z = (9*Y - (15*var_V*Y) - (var_V*X)) / (3*var_V);

  return (XYZ) {X,Y,Z};
}

RGB xyz2rgb (XYZ c) {
  // http://www.easyrgb.com/index.php?X=MATH&H=01
  const float gamma = 1/2.4;
  const float thresh = 0.0031308f;
  const float meet = 0.055f;
  const float linear = 12.92f;

  const float var_X = c.X / 100;  // X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
  const float var_Y = c.Y / 100;  // Y from 0 to 100.000
  const float var_Z = c.Z / 100;  // Z from 0 to 108.883

  float var_R = var_X *  3.2406  +  var_Y * -1.5372  +  var_Z * -0.4986;
  float var_G = var_X * -0.9689  +  var_Y *  1.8758  +  var_Z *  0.0415;
  float var_B = var_X *  0.0557  +  var_Y * -0.2040  +  var_Z *  1.0570;

  if (var_R > thresh) var_R = (1+meet)*pow(var_R, gamma) - meet;
  else                var_R = linear * var_R;
  if (var_G > thresh) var_G = (1+meet)*pow(var_G, gamma) - meet;
  else                var_G = linear * var_G;
  if (var_B > thresh) var_B = (1+meet)*pow(var_B, gamma) - meet;
  else                var_B = linear * var_B;

  // RGB needs clipping
  // if (var_R > 1) var_R = 1;
  // if (var_G > 1) var_G = 1;
  // if (var_B > 1) var_B = 1;

  return (RGB){
    var_R * 255,
    var_G * 255,
    var_B * 255
  };
}


SDL_FColor colour_luv2rgb(Colour_Luv Luv) {
  RGB col = xyz2rgb(luv2xyz(Luv));
  return (SDL_FColor) {col.r/255, col.g/255, col.b/255, SDL_ALPHA_OPAQUE};
}