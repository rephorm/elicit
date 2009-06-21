#ifndef ELICIT_SHOT_H
#define ELICIT_SHOT_H

typedef struct Elicit_Shot_Event_Selection Elicit_Shot_Event_Selection;
typedef void (*Elicit_Shot_Callback_Func) (void *data, Elicit_Shot_Event_Selection *event);

struct Elicit_Shot_Event_Selection
{
  int w, h;
  float length;
};

Evas_Object *elicit_shot_add(Evas *evas);
void elicit_shot_zoom_set(Evas_Object *o, int zoom);
void elicit_shot_grid_visible_set(Evas_Object *o, int visible);
void elicit_shot_grab(Evas_Object *o, int x, int y, int w, int h, int force);
void elicit_shot_data_get(Evas_Object *o, void **data, int *w, int *h);
void elicit_shot_data_set(Evas_Object *o, void *data, int w, int h);

void elicit_shot_size_get(Evas_Object *o, int *w, int *h);

void elicit_shot_callback_select_add(Evas_Object *obj, Elicit_Shot_Callback_Func func, void *data);
void elicit_shot_callback_select_del(Evas_Object *obj, Elicit_Shot_Callback_Func func);

#endif
