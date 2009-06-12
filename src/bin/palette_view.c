#include "Elicit.h"
#include "palette_view.h"

static Evas_Smart *pv_smart;
Evas_Smart_Class pv_class;


static void pv_init();
static void pv_add(Evas_Object *obj);
static void pv_del(Evas_Object *obj);
static void pv_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void pv_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void pv_clip_set(Evas_Object *obj, Evas_Object *clip);
static void pv_clip_unset(Evas_Object *obj);

static void pv_layout(Evas_Object *obj);
static int pv_layout_timer(void *data);

static void cb_swatch_in(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void cb_swatch_out(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void cb_swatch_up(void *data, Evas *evas, Evas_Object *obj, void *event_info);

#undef API_ENTRY
#define API_ENTRY \
  Palette_View *pv; \
  pv = evas_object_smart_data_get(obj); \
  if (!pv) return

/** API **/
Evas_Object *
palette_view_add(Evas *evas)
{
  pv_init();
  return evas_object_smart_add(evas, pv_smart);
}

void
palette_view_palette_set(Evas_Object *obj, Palette *palette)
{
  API_ENTRY;

  pv->palette = palette;
  pv_layout(obj);
}

Palette *
palette_view_palette_get(Evas_Object *obj)
{
  API_ENTRY;

  return pv->palette;
}

void
palette_view_changed(Evas_Object *obj)
{
  API_ENTRY;
  pv_layout(obj);
}


static void
pv_init()
{
  if (pv_smart) return;
  pv_class.name = "Palette_View";
  pv_class.version = EVAS_SMART_CLASS_VERSION;
  pv_class.add = pv_add;
  pv_class.del = pv_del;
  pv_class.move = pv_move;
  pv_class.resize = pv_resize;
  pv_class.clip_set = pv_clip_set;
  pv_class.clip_unset = pv_clip_unset;
  pv_smart = evas_smart_class_new(&pv_class);
}


static void
pv_add(Evas_Object *obj)
{
  Palette_View *pv;

  pv = calloc(1, sizeof(Palette_View));
  pv->smart_obj = obj;
  pv->rects = NULL;
  pv->size = 20;

  evas_object_smart_data_set(obj, pv);
}

static void
pv_del(Evas_Object *obj)
{
  API_ENTRY;
  // XXX free rects themselves??
  eina_list_free(pv->rects);
  free(pv);
}

static void
pv_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
  API_ENTRY;
  if (pv->x == x && pv->y == y) return;
  pv->x = x;
  pv->y = y;
  pv_layout(obj);
}

static void
pv_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
  API_ENTRY;
  if (pv->w == w && pv->h == h) return;
  pv->w = w;
  pv->h = h;
  pv_layout(obj);
}

static void
pv_clip_set(Evas_Object *obj, Evas_Object *clip)
{
  Eina_List *l;
  Evas_Object *rect;
  API_ENTRY;

  EINA_LIST_FOREACH(pv->rects, l, rect) {
    evas_object_clip_set(rect, clip);
  }
}

static void
pv_clip_unset(Evas_Object *obj)
{
  Eina_List *l;
  Evas_Object *rect;
  API_ENTRY;

  EINA_LIST_FOREACH(pv->rects, l, rect) {
    evas_object_clip_unset(rect);
  }
}

static void
pv_layout(Evas_Object *obj)
{
  API_ENTRY;
  pv->layout_timer = ecore_timer_add(0.01, pv_layout_timer, pv);
}

static int
pv_layout_timer(void *data)
{
  Evas_Object *rect;
  Eina_List *colors, *lc, *lr, *lr_next;
  Color *c;
  int cols, i;
  Evas_Object *clip;
  int x, y, w, h, adj;
  Palette_View *pv;

  pv = data;
  if (!pv) return 0;

  x = y = w = h = adj = 0;

  lr = NULL;
  if (pv->palette) 
  {
    clip = evas_object_clip_get(pv->smart_obj);

    colors = palette_colors_get(pv->palette);
    cols = palette_columns_get(pv->palette);
    if (cols == 0) cols = 1; //XXX use a global config value instead

    w = pv->w / cols;
    h = pv->size;

    adj = (pv->w - (w * cols)) / 2;

    i = 0;
    lr = pv->rects;
    EINA_LIST_FOREACH(colors, lc, c)
    {
      int r,g,b;

      rect = eina_list_data_get(lr);
      if (!rect)
      {
        rect = edje_object_add(evas_object_evas_get(pv->smart_obj));
        evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_IN, cb_swatch_in, pv);
        evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_OUT, cb_swatch_out, pv);
        evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, cb_swatch_up, pv);
        if (clip) evas_object_clip_set(rect, clip);
        elicit_theme_edje_object_set(rect, "elicit/swatch2");

        pv->rects = eina_list_append(pv->rects, rect);
      }

      color_rgba_get(c, &r, &g, &b, NULL);
      edje_object_color_class_set(rect, "swatch", r, g, b, 255, 0, 0, 0, 0, 0, 0, 0, 0);

      x = pv->x + w * (i % cols) + (i % cols ? adj : 0);
      y = pv->y + h * (i / cols);

      evas_object_move(rect, x, y);
      // left most gets corrected by 'adj', rightmost is corrected by adj + 1 to fill space (they are clipped, so going over is ok)
      evas_object_resize(rect, w + ((i % cols == 0) ? adj : (i % cols == cols - 1) ? adj + 1 : 0), h);
      evas_object_show(rect);

      evas_object_data_set(rect, "Color", c);
      evas_object_smart_member_add(rect, pv->smart_obj);

      //printf("place %s at (%d,%d) %d x %d\n", color_hex_get(c, 1), x, y, w, h);
      i++;
      lr = eina_list_next(lr);
    } 
    evas_object_resize(pv->smart_obj, pv->w, y + h - pv->y);
  }

  // remove any leftover rects from a previous larger palette
  while (lr)
  {
    lr_next = eina_list_next(lr);
    rect = eina_list_data_get(lr);
    evas_object_smart_member_del(rect);
    evas_object_del(rect);
    pv->rects = eina_list_remove_list(pv->rects, lr);
    lr = lr_next;
  }

  pv->layout_timer = NULL;
  return 0;
}


static void
cb_swatch_in(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Color *c;
  c = evas_object_data_get(obj, "Color");
  if (!c) return;
}

static void
cb_swatch_out(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Color *c;
  c = evas_object_data_get(obj, "Color");
  if (!c) return;
}

static void
cb_swatch_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Up *ev;
  Palette_View *pv;
  Color *c;

  ev = event_info;
  pv = data; 
  c = evas_object_data_get(obj, "Color");
  if (!c) return;

  if (ev->button == 1)
    evas_object_smart_callback_call(pv->smart_obj, "selected", c);
  else if (ev->button == 3)
  {
    palette_color_remove(pv->palette, c);
    palette_view_changed(pv->smart_obj);
    evas_object_smart_callback_call(pv->smart_obj, "deleted", c);
  }
}

