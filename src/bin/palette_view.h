#ifndef PALETTE_VIEW_H
#define PALETTE_VIEW_H

#include "Elicit.h"

typedef struct Palette_View Palette_View;

struct Palette_View
{
  Evas_Object *smart_obj;
  Palette *palette;
  Eina_List *rects;

  Evas_Coord x,y;
  Evas_Coord w,h;

  int size;
  int direction : 1;

  Ecore_Timer *layout_timer;
};

Evas_Object *palette_view_add(Evas *evas);
void palette_view_palette_set(Evas_Object *obj, Palette *palette);
Palette *palette_view_palette_get(Evas_Object *obj);
void palette_view_changed(Evas_Object *obj);

#endif
