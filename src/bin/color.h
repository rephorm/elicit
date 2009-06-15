#ifndef ELICIT_COLOR_H
#define ELICIT_COLOR_H

#define COLOR_HEX_HASH 1
#define COLOR_HEX_ALPHA 2

typedef struct Color Color;
typedef enum Color_Mode Color_Mode;
enum Color_Mode
{
  COLOR_MODE_RGBA,
  COLOR_MODE_HSVA
};

struct Color
{
  const char *name;
  Color_Mode mode;
  int r, g, b;
  float h, s, v;
  int a;
};

Color *color_new();
void color_free(Color *color);
Color *color_clone(Color *color);
void color_copy(Color *from, Color *to);

void color_name_set(Color *color, const char *name);
const char *color_name_get(Color *color);

Color_Mode color_mode_get(Color *color);
void color_rgba_set(Color *color, int r, int g, int b, int a);
void color_hsva_set(Color *color, float h, float s, float v, int a);
void color_argb_int_set(Color *color, int argb);

void color_rgba_get(Color *color, int *r, int *g, int *b, int *a);
void color_hsva_get(Color *color, float *h, float *s, float *v, int *a);
int  color_argb_int_get(Color *color);

void color_hex_set(Color *color, const char *hex);
const char *color_hex_get(Color *color, int options);

#endif
