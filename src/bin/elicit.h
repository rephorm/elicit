#ifndef ELICIT_H
#define ELICIT_H

#include <Evas.h>
#include <Ecore_Evas.h>
#include <Eina.h>
#include <Edje.h>

#include <limits.h>
#include <string.h>

#include "palette.h"
#include "color.h"

struct Elicit {
  Ecore_Evas *ee;
  Evas *evas;

  struct
  {
    Evas_Object *main;
    Evas_Object *swatch, *shot;
    Evas_Object *draggie;
    Evas_Object *shot_list, *swatch_list, *palette_list;
  } obj;

  Color *color;
  Palette *palette;

  Eina_List *palettes;

  struct
  {
    char shooting : 1;
    char picking  : 1;
  } state;

  struct {
    int x, y, w, h;

    char *theme;

    int zoom_level;
    int grid_visible;

    int changed;
  } conf;
};

typedef struct Elicit Elicit;

Elicit *elicit_new();
void elicit_free(Elicit *el);
void elicit_show(Elicit *el);
void elicit_hide(Elicit *el);
int elicit_theme_set(Elicit *el, const char *theme);
int  elicit_libs_init(void);
void elicit_libs_shutdown();

#endif
