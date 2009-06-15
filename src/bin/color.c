#include <stdlib.h>
#include <stdio.h>
#include <Eina.h>
#include <Evas.h>
#include "color.h"

/**
 * Create a new color
 */
Color *
color_new()
{
  Color *color;

  color = calloc(1, sizeof(Color));
  if (!color) return NULL;

  color->r = color->g = color->b = color->a = 255;
  color->h = 0; color->s = 1; color->v = 1;
  return color;
}

/**
 * Free a color
 * @param color - the color to free
 */
void 
color_free(Color *color)
{
  if (!color) return;
  if (color->name) eina_stringshare_del(color->name);
  free(color);
}

/**
 * Clone a color (make a deep copy of it)
 * @param color - the color to clone
 * @return - the clone
 */
Color *
color_clone(Color *color)
{
  Color *clone;
  clone = color_new();
  color_copy(color, clone);
  return clone;
}

/**
 * Copy the color values from one color struct to another
 * @param from
 * @param to
 */
void color_copy(Color *from, Color *to)
{
  if (from->name)
    to->name = eina_stringshare_add(from->name);

  to->mode = from->mode;
  to->r = from->r;
  to->g = from->g;
  to->b = from->b;
  to->h = from->h;
  to->s = from->s;
  to->v = from->v;
  to->a = from->a;
}

/**
 * Set a color's name
 * @param color
 * @param name
 */
void 
color_name_set(Color *color, const char *name)
{
  if (color->name) eina_stringshare_del(color->name);
  if (name)
    color->name = eina_stringshare_add(name);
  else
    color->name = NULL;
}

/**
 * Get a color's name 
 * @param color
 * @return the name or "" if none is set
 */
const char *
color_name_get(Color *color)
{
  return color->name ? color->name : "";
}

/**
 * Get a color's mode
 * @param color
 * @return the mode (color space) the color was specified in
 */
Color_Mode
color_mode_get(Color *color)
{
  return color->mode;
}

/**
 * Set a color from RGBA values
 * @param color
 * @param r - red (0-255)
 * @param g - green (0-255)
 * @param b - blue (0-255)
 * @param a - alpha (0-255)
 *
 * If -1 is passed for r, g, b, or a, then that value will not be changed.
 */
void
color_rgba_set(Color *color, int r, int g, int b, int a)
{
  color->mode = COLOR_MODE_RGBA;

  if (r >= 0) color->r = r;
  if (g >= 0) color->g = g;
  if (b >= 0) color->b = b;
  if (a >= 0) color->a = a;
  evas_color_rgb_to_hsv(color->r, color->g, color->b, &(color->h), &(color->s), &(color->v));
}

/**
 * Set a color from HSVA values
 * @param color
 * @param h - the hue (0-360)
 * @param s - the saturation (0.0 - 1.0)
 * @param v - the value (0.0 - 1.0)
 * @param a - alpha (0-255)
 *
 * If -1 is passed for h, s, v, or a, then that value will not be changed.
 */
void
color_hsva_set(Color *color, float h, float s, float v, int a)
{
  color->mode = COLOR_MODE_HSVA;
  if (h >= 0) color->h = h;
  if (s >= 0) color->s = s;
  if (v >= 0) color->v = v;
  if (a >= 0) color->a = a;
  evas_color_hsv_to_rgb(color->h, color->s, color->v, &(color->r), &(color->g), &(color->b));
}

/**
 * Set a color from ARGB values packed into an integer
 * @param color
 * @param argb
 */
void
color_argb_int_set(Color *color, int argb)
{
  color_rgba_set(
    color,
    argb >> 16 & 0xff,
    argb >> 8  & 0xff,
    argb       & 0xff,
    argb >> 24 & 0xff
  );
}

/**
 * Get the RGBA values of a color
 * @param color
 * @param r - an int pointer to return the red value in 
 * @param g - an int pointer to return the green value in 
 * @param b - an int pointer to return the blue value in 
 * @param a - an int pointer to return the alpha value in 
 *
 * Any return pointers which are NULL will be skipped.
 */
void
color_rgba_get(Color *color, int *r, int *g, int *b, int *a)
{
  if (r) *r = color->r;
  if (g) *g = color->g;
  if (b) *b = color->b;
  if (a) *a = color->a;
}

/**
 * Get the HSVA values of a color
 * @param color
 * @param h - a float pointer to return the hue value in
 * @param s - a float pointer to return the saturation value in
 * @param v - a float pointer to return the valuee value in 
 * @param a - an int pointer to return the alpha value in
 *
 * Any return pointers which are NULL will be skipped.
 */
void
color_hsva_get(Color *color, float *h, float *s, float *v, int *a)
{
  if (h) *h = color->h;
  if (s) *s = color->s;
  if (v) *v = color->v;
  if (a) *a = color->a;
}

/**
 * Get the ARGB values of a color as a packed integer
 * @param color
 * @return - rgba packed in to integer  
 */
int
color_argb_int_get(Color *color)
{
  return color->a << 24 |
         color->r << 16 |
         color->g << 8 |
         color->b;
}

/**
 * Set a color from a hex string
 * @param color
 * @param hex
 *
 * The hex string consists of an optional hash mark ('#') followed by 2 digits each of r, g, b and optionally, a.
 * The following are all valid for 'opaque white' (r = g = b = a = 255):
 *   ffffff 
 *   #ffffff
 *   ffffffff
 *   #ffffffff
 *
 * (if the alpha value is left off, 255 is implied)
 */
void
color_hex_set(Color *color, const char *hex)
{
  int r, g, b;
  if (hex[0] == '#') hex++;
  if (sscanf(hex, "%2x%2x%2x", &r, &g, &b)) {
    color_rgba_set(color, r, g, b, color->a);
  }
}

/**
 * Get the RGB(A) value of a color as a hex string with an optional 
 * leading '#'
 * @param color
 * @param with_hash - whether to include leading '#' or not
 * @param with_alpha - whether to include alpha values or not
 */
const char *
color_hex_get(Color *color, int options)
{
  static char buf[10];
  char *formats[4] = {
    "%02x%02x%02x",
    "#%02x%02x%02x",
    "%02x%02x%02x%02x",
    "#%02x%02x%02x%02x"
  };

  sprintf(buf, formats[options & 0x3], color->r, color->g, color->b, color->a);
  return buf;
}


