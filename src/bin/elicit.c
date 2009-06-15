#include <X11/Xlib.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_File.h>
#include "elicit.h"
#include "config.h"
#include "zoom.h"

void
_elicit_cb_resize(Ecore_Evas *ee)
{
  Elicit *el;
  int w, h;

  el = ecore_evas_data_get(ee, "Elicit");
  if (!el) return;

  ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

  if (el->conf.w == w && el->conf.h == h) return;

  if (el->obj.main)
    evas_object_resize(el->obj.main, w, h);

  el->conf.w = w;
  el->conf.h = h;
  el->conf.changed = 1;
}

void
_elicit_cb_edje_signal(void *data, Evas_Object *obj, const char *emission, const char *source)
{
  Elicit *el = data;
  char *signal;
  char *tok;

  printf("signal (%s)!\n", emission);

  signal = strdup(emission);

  tok = strtok(signal, ","); // first is 'elicit'
  tok = strtok(NULL, ",");

  if (!tok) {
    fprintf(stderr, "[Elicit] Error: invalid signal: %s\n", emission);
    free(signal);
    return;
  }

  if (!strcmp(tok, "shoot"))
  {
    tok = strtok(NULL, ",");
    if (!strcmp(tok, "start"))
    {
      if (!el->band)
        el->band = elicit_band_new();

      elicit_band_show(el->band);
      el->state.shooting = 1;
    }
    else if (!strcmp(tok, "stop"))
    {
      elicit_band_hide(el->band);
      el->state.shooting = 0;
    }
    else
      fprintf(stderr, "[Elicit] Error: invalid signal: %s\n", emission);
  }

  free(signal);
}

void
_elicit_cb_edje_move(void *data, Evas_Object *obj, const char *emission, const char *source)
{
  Elicit *el = data;


  if (el->state.shooting)
  {
    elicit_shoot(el);
  }

  if (el->state.picking)
  {
  }
}

void
elicit_shoot(Elicit *el)
{
  int x, y;
  int px, py;
  int dw, dh;
  int w, h;

  ecore_x_pointer_last_xy_get(&px, &py);
  elicit_zoom_size_get(el->obj.shot, &w, &h);

  x = px - .5 * w;
  y = py - .5 * h;

  /* keep shot within desktop bounds */
  ecore_x_window_size_get(RootWindow(ecore_x_display_get(),0), &dw, &dh);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x + w > dw) x = dw - w;
  if (y + h > dh) y = dh - h;

  if (elicit_config_show_band_get(el))
    elicit_band_move_resize(el->band, x-1, y-1, w+2, h+2);
  elicit_zoom_grab(el->obj.shot, x, y, w, h, 0);
  evas_render(el->evas);
  
}


Elicit *
elicit_new()
{
  Elicit *el;
  
  el = calloc(sizeof(Elicit), 1);

  el->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 500, 500);
  el->evas = ecore_evas_get(el->ee);

  ecore_evas_title_set(el->ee, "Elicit");
  ecore_evas_name_class_set(el->ee, "Elicit", "Elicit");
  ecore_evas_borderless_set(el->ee, 1);
  ecore_evas_shaped_set(el->ee, 1);

  ecore_evas_data_set(el->ee, "Elicit", el);
  ecore_evas_callback_resize_set(el->ee, _elicit_cb_resize);

  el->obj.main = edje_object_add(el->evas);

  el->color = color_new();

  return el;
}

void
elicit_free(Elicit *el)
{
  if (el->color)
    color_free(el->color);

  if (el->obj.main)
    evas_object_del(el->obj.main);

  if (el->obj.shot)
    evas_object_del(el->obj.shot);

  if (el->obj.swatch)
    evas_object_del(el->obj.swatch);

  if (el->ee)
    ecore_evas_free(el->ee);

  if (el->band)
    elicit_band_free(el->band);

  if (el->conf.theme)
    free(el->conf.theme);

  free(el);
}

void
elicit_show(Elicit *el)
{
  ecore_evas_show(el->ee);
}

void
elicit_hide(Elicit *el)
{
  ecore_evas_hide(el->ee);
}

const char *
elicit_theme_find(const char *theme)
{
  static char buf[PATH_MAX];

  // XXX run over set of dirs
  snprintf(buf, sizeof(buf), "/home/rephorm/code/elicit2/data/themes/default/%s.edj", theme);
  if (ecore_file_exists(buf))
    return buf;
  else
    return NULL;
}


void
elicit_theme_swallow_objs(Elicit *el)
{

  if (edje_object_part_exists(el->obj.main, "elicit.shot"))
  {
    //XXX wrap in pan widget?
    if (!el->obj.shot)
    {
      el->obj.shot = elicit_zoom_add(el->evas);
      elicit_zoom_zoom_set(el->obj.shot, elicit_config_zoom_level_get(el));
      elicit_zoom_grid_visible_set(el->obj.shot, elicit_config_grid_visible_get(el));
    }

    edje_object_part_swallow(el->obj.main, "elicit.shot", el->obj.shot);

  }
  else
  {
    if (el->obj.shot)
    {
      evas_object_del(el->obj.shot);
      el->obj.shot = NULL;
    }
  }


  if (edje_object_part_exists(el->obj.main, "elicit.swatch"))
  {
    if (!el->obj.swatch)
    {
      el->obj.swatch = evas_object_rectangle_add(el->evas);
      evas_object_color_set(el->obj.swatch, 255, 255, 255, 255);
      evas_object_show(el->obj.swatch);
    }

    edje_object_part_swallow(el->obj.main, "elicit.swatch", el->obj.swatch);
  }
  else
  {
    if (el->obj.swatch)
    {
      evas_object_del(el->obj.swatch);
      el->obj.swatch = NULL;
    }
  }
}

void
elicit_theme_unswallow_objs(Elicit *el)
{
  if (!el->obj.main)
    return;

  if (el->obj.shot)
    edje_object_part_unswallow(el->obj.main, el->obj.shot);

  if (el->obj.swatch)
    edje_object_part_unswallow(el->obj.main, el->obj.swatch);
}

int
elicit_theme_set(Elicit *el, const char *theme)
{
  const char *path;
  int w, h;

  path = elicit_theme_find(theme);

  if (!path)
  {
    fprintf(stderr, "[Elicit] Error: theme \"%s\" not found.\n", theme);
    return 0;
  }

  if (el->conf.theme) free(el->conf.theme);
  el->conf.theme = strdup(theme);

  elicit_theme_unswallow_objs(el);

  edje_object_file_set(el->obj.main, path, "elicit.main");

  edje_object_size_min_get(el->obj.main, &w, &h);
  if (w > el->conf.w) el->conf.w = w;
  if (h > el->conf.h) el->conf.h = h;
  ecore_evas_size_min_set(el->ee, w, h);
  ecore_evas_resize(el->ee, el->conf.w, el->conf.h);

  evas_object_move(el->obj.main, 0, 0);
  evas_object_resize(el->obj.main, el->conf.w, el->conf.h);
  evas_object_show(el->obj.main);

  elicit_theme_swallow_objs(el);

  edje_object_signal_callback_add(el->obj.main, "elicit,*", "*", _elicit_cb_edje_signal, el);
  edje_object_signal_callback_add(el->obj.main, "mouse,move", "*", _elicit_cb_edje_move, el);

  return 1;
}

/**
 * Initialize libraries
 */
int
elicit_libs_init(void)
{
  if (!eina_init())
  {
    fprintf(stderr, "[Elicit] Failed to initialize eina.\n");
    return 0;
  }
  if (!ecore_init())
  {
    fprintf(stderr, "[Elicit] Failed to initialize ecore.\n");
    eina_shutdown();
    return 0;
  }
  if (!ecore_evas_init())
  {
    fprintf(stderr, "[Elicit] Failed to initialize ecore_evas.\n");
    eina_shutdown();
    ecore_shutdown();
    return 0;
  }
  if (!ecore_file_init())
  {
    fprintf(stderr, "[Elicit] Failed to initialize ecore_file.\n");
    eina_shutdown();
    ecore_shutdown();
    ecore_evas_shutdown();
    return 0;
  }
  if (!edje_init())
  {
    fprintf(stderr, "[Elicit] Failed to initialize edje.\n");
    eina_shutdown();
    ecore_shutdown();
    ecore_evas_shutdown();
    ecore_file_shutdown();
    return 0;
  }

  return 1;
}

/**
 * Shutdown libraries
 */
void
elicit_libs_shutdown()
{
  edje_shutdown();
  ecore_file_shutdown();
  ecore_evas_shutdown();
  ecore_shutdown();
  eina_shutdown();
}


