#include <X11/Xlib.h>
#include <Ecore_File.h>
#include "elicit.h"
#include "zoom.h"
#include "config.h"
#include "grab.h"

static Evas_Smart *_smart;

typedef struct _Elicit_Zoom Elicit_Zoom;

struct _Elicit_Zoom
{
  Evas_Object *shot;
  Evas_Object *grid;
  Evas_Object *clip;

  Evas_Coord ow, oh; // current object size
  int iw, ih; // how large of a shot to take
  
  struct { int x, y, w, h; } cur; //location on screen that current shot came from

  int zoom;
  char grid_visible : 1;

  int has_data;
};

static void _smart_init(void);
static void _smart_add(Evas_Object *o);
static void _smart_del(Evas_Object *o);
static void _smart_move(Evas_Object *o, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *o, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *o);
static void _smart_hide(Evas_Object *o);
static void _smart_color_set(Evas_Object *o, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *o, Evas_Object *clip);
static void _smart_clip_unset(Evas_Object *o);


Evas_Object *
elicit_zoom_add(Evas *evas) 
{
  _smart_init();
  return evas_object_smart_add(evas, _smart);
}

void
elicit_zoom_zoom_set(Evas_Object *o, int zoom)
{
  Elicit_Zoom *z;
  int iw, ih;

  z = evas_object_smart_data_get(o);
  if (z->zoom == zoom) return;

  z->zoom = zoom;
  evas_object_image_size_get(z->shot, &iw, &ih);
  evas_object_image_fill_set(z->shot, 0, 0, iw * zoom, ih * zoom);

  evas_object_image_fill_set(z->grid, 0, 0, zoom, zoom);
}

void
elicit_zoom_size_get(Evas_Object *o, int *w, int *h)
{
  Elicit_Zoom *z;
  z = evas_object_smart_data_get(o);
  //XXX cache this calc on resize or zoom level change?
  if (w) *w = (z->ow / z->zoom) + (z->ow % z->zoom ? 1 : 0);
  if (h) *h = (z->oh / z->zoom) + (z->oh % z->zoom ? 1 : 0);
}

void
elicit_zoom_grid_visible_set(Evas_Object *o, int visible)
{
  Elicit_Zoom *z;
  Evas_Object *gui;
  z = evas_object_smart_data_get(o);

  if (z->grid_visible == visible)
    return;

  z->grid_visible = visible;
  gui = evas_object_name_find(evas_object_evas_get(o), "gui");

  if (visible)
  {
    if (z->has_data) evas_object_show(z->grid);
    if (gui) edje_object_signal_emit(gui, "elicit,grid,shown", "Elicit");
  }
  else
  {
    if (z->has_data) evas_object_hide(z->grid);
    if (gui) edje_object_signal_emit(gui, "elicit,grid,hidden", "Elicit");
  }

}

void
elicit_zoom_grab(Evas_Object *o, int x, int y, int w, int h, int force)
{
  Elicit_Zoom *z;
  void *data;

  z = evas_object_smart_data_get(o);

  // don't grab same region twice
  if (!force && (x == z->cur.x && y == z->cur.y && w == z->cur.w && h == z->cur.h))
    return;

  z->iw = w;
  z->ih = h;

  evas_object_image_alpha_set(z->shot, 0);
  evas_object_image_size_set(z->shot, z->iw, z->ih);
  evas_object_image_smooth_scale_set(z->shot, 0);

  data = evas_object_image_data_get(z->shot, 1);

  if (!elicit_grab_region(x, y, z->iw, z->ih, 0, data))
  {
    fprintf(stderr, "[Elicit] Error: can't grab region\n");
    return;
  }

  evas_object_image_data_set(z->shot, data);
  evas_object_image_data_update_add(z->shot, 0, 0, z->iw, z->ih);

  /* set it to fill at the current zoom level */
  evas_object_image_fill_set(z->shot, 0, 0, z->iw * z->zoom, z->ih * z->zoom);
  evas_object_resize(z->shot, z->iw * z->zoom, z->ih * z->zoom);
  evas_object_show(z->shot);
  if (z->grid_visible)
    evas_object_show(z->grid);

  z->has_data = 1;
}

void
elicit_zoom_data_get(Evas_Object *o, void **data, int *w, int *h) 
{
  Elicit_Zoom *z;
  int iw, ih;

  z = evas_object_smart_data_get(o);
  if (!z) return;

  evas_object_image_size_get(z->shot, &iw, &ih);
  if (w) *w = iw;
  if (h) *h = ih;
  if (data) *data = evas_object_image_data_get(z->shot, 1);
}

void
elicit_zoom_data_set(Evas_Object *o, void *data, int w, int h)
{
  Elicit_Zoom *z;

  z = evas_object_smart_data_get(o);
  if (!z) return;

  z->iw = w;
  z->ih = h;

  /* get the object ready, copy the data in */
  evas_object_image_alpha_set(z->shot, 0);
  evas_object_image_size_set(z->shot, z->iw, z->ih);
  evas_object_image_smooth_scale_set(z->shot, 0);
  evas_object_image_data_copy_set(z->shot, data);
  
  /* tell evas that we changed part of the image data */
  evas_object_image_data_update_add(z->shot, 0, 0, z->iw, z->ih);

  /* set it to fill at the current zoom level */
  evas_object_image_fill_set(z->shot, 0, 0, z->iw * z->zoom, z->ih * z->zoom);
  evas_object_resize(z->shot, z->iw * z->zoom, z->ih * z->zoom);

  z->has_data = 1;
}


static void
_smart_init(void)
{
  static const Evas_Smart_Class sc =
  {
    "elicit_zoom",
    EVAS_SMART_CLASS_VERSION,
    _smart_add,
    _smart_del,
    _smart_move,
    _smart_resize,
    _smart_show,
    _smart_hide,
    _smart_color_set,
    _smart_clip_set,
    _smart_clip_unset,
    NULL,
    NULL,
    NULL,
    NULL
  };

  if (_smart) return;
  _smart = evas_smart_class_new(&sc);
}

static void
_smart_add(Evas_Object *o)
{
  Elicit_Zoom *z;
  Evas *evas;
  char buf[PATH_MAX];
  char *datadir;

  z = calloc(1, sizeof(Elicit_Zoom));
  if (!z) return;

  evas_object_smart_data_set(o, z);

  evas = evas_object_evas_get(o);

  z->shot = evas_object_image_add(evas);
  evas_object_smart_member_add(z->shot, o);

  z->grid = evas_object_image_add(evas);

  datadir = br_find_data_dir(DATADIR);
  snprintf(buf, sizeof(buf), "%s/%s/images/grid_cell.png", datadir, PACKAGE);
  free(datadir);
  if (!ecore_file_exists(buf))
    snprintf(buf, sizeof(buf), "%s/%s/images/grid_cell.png", DATADIR, PACKAGE);

  evas_object_image_file_set(z->grid, buf, "");
  evas_object_smart_member_add(z->grid, o);

  z->clip = evas_object_rectangle_add(evas);
  evas_object_clip_set(z->shot, z->clip);
  evas_object_clip_set(z->grid, z->clip);

  elicit_zoom_zoom_set(o, 4);
}

static void
_smart_del(Evas_Object *o)
{
  Elicit_Zoom *z;

  z = evas_object_smart_data_get(o);

  if (z->shot) evas_object_del(z->shot);
  if (z->grid) evas_object_del(z->grid);
  if (z->clip) evas_object_del(z->clip);

  free(z);
  evas_object_smart_data_set(o, NULL);
}

static void
_smart_move(Evas_Object *o, Evas_Coord x, Evas_Coord y)
{
  Elicit_Zoom *z;

  z = evas_object_smart_data_get(o);

  evas_object_move(z->grid, x, y);
  evas_object_move(z->shot, x, y);
  evas_object_move(z->clip, x, y);
}

static void
_smart_resize(Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
  Elicit_Zoom *z;

  z = evas_object_smart_data_get(o);
  evas_object_resize(z->shot, z->iw * z->zoom, z->ih * z->zoom);
  evas_object_resize(z->grid, w, h);
  evas_object_resize(z->clip, w, h);

  z->ow = w;
  z->oh = h;
}

static void
_smart_show(Evas_Object *o)
{
  Elicit_Zoom *z;

  z = evas_object_smart_data_get(o);

  evas_object_show(z->shot);
  evas_object_show(z->clip);
  if (z->has_data && z->grid_visible)
    evas_object_show(z->grid);
  else
    evas_object_hide(z->grid);
}

static void
_smart_hide(Evas_Object *o)
{
  Elicit_Zoom *z;

  z = evas_object_smart_data_get(o);

  evas_object_hide(z->shot);
  evas_object_hide(z->grid);
  evas_object_hide(z->clip);
}

static void
_smart_color_set(Evas_Object *o, int r, int g, int b, int a)
{
  Elicit_Zoom *z;

  z = evas_object_smart_data_get(o);

  evas_object_color_set(z->clip, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *o, Evas_Object *clip)
{
  Elicit_Zoom *z;
  
  z = evas_object_smart_data_get(o);

  evas_object_clip_set(z->clip, clip);
}

static void
_smart_clip_unset(Evas_Object *o)
{
  Elicit_Zoom *z;
  
  z = evas_object_smart_data_get(o);

  evas_object_clip_unset(z->clip);
}

