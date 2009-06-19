#ifndef ELICIT_H
#define ELICIT_H

#include <Evas.h>
#include <Ecore_Evas.h>
#include <Eina.h>
#include <Edje.h>

#include <limits.h>
#include <string.h>
#include "sysconfig.h"
#include "binreloc.h"

#include "palette.h"
#include "color.h"
#include "band.h"

struct Elicit {
  Ecore_Evas *ee;
  Evas *evas;

  struct
  {
    Evas_Object *main;
    Evas_Object *swatch, *shot;
    Evas_Object *draggie;
    Evas_Object *shot_list, *swatch_list, *palette, *palette_frame;

    Evas_Object *cslider[6];

    Evas_Object *related[3];
  } obj;

  Color *color;
  Palette *palette;

  Elicit_Band *band;

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
    unsigned char grid_visible : 1;
    unsigned char show_band : 1;

    int changed;
  } conf;

  struct
  {
    char *datadir;
    char *theme;
    char *palette;
  } path;
};

typedef struct Elicit Elicit;

Elicit *elicit_new();
void elicit_free(Elicit *el);
void elicit_show(Elicit *el);
void elicit_hide(Elicit *el);
int elicit_theme_set(Elicit *el, const char *theme);
int  elicit_libs_init(void);
void elicit_libs_shutdown();

void elicit_shoot(Elicit *el);
void elicit_pick(Elicit *el);
void elicit_color_update(Elicit *el);
void elicit_scroll(Elicit *el, const char *source, int dir);

#define IF_FREE(x) if (x) free(x);
#endif
