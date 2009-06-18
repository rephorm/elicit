#include <X11/Xlib.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_File.h>
#include "elicit.h"
#include "config.h"
#include "zoom.h"
#include "grab.h"
#include "color.h"
#include "cslider.h"

static char *cslider_part_names[6] = {
  "elicit.cslider.red",
  "elicit.cslider.green",
  "elicit.cslider.blue",
  "elicit.cslider.hue",
  "elicit.cslider.saturation",
  "elicit.cslider.value"
};

static char *related_color_names[3] = {
  "elicit.color.triad1",
  "elicit.color.complement",
  "elicit.color.triad2"
};

static int related_color_offset[3] = { 120, 180, 240 };

static void
elicit_cb_ee_resize(Ecore_Evas *ee)
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

static void
elicit_cb_ee_mouse_in(Ecore_Evas *ee)
{
  Elicit *el;
  el = ecore_evas_data_get(ee, "Elicit");
  if (!el) return;

  edje_object_signal_emit(el->obj.main, "elicit,activate", "elicit");
}

static void
elicit_cb_ee_mouse_out(Ecore_Evas *ee)
{
  Elicit *el;
  el = ecore_evas_data_get(ee, "Elicit");
  if (!el) return;

  edje_object_signal_emit(el->obj.main, "elicit,deactivate", "elicit");
}

void
_elicit_cb_edje_signal(void *data, Evas_Object *obj, const char *emission, const char *source)
{
  Elicit *el = data;
  char *signal;
  char *tok;
  int invalid = 0;

  /* ignore signals we sent */
  if (!strcmp(source, "elicit"))
    return;

  signal = strdup(emission);

  tok = strtok(signal, ","); // first is 'elicit'
  tok = strtok(NULL, ",");

  if (!tok) {
    fprintf(stderr, "[Elicit] Error: invalid signal: %s\n", emission);
    free(signal);
    return;
  }

  /* magnification */
  if (!strcmp(tok, "shoot"))
  {
    tok = strtok(NULL, ",");
    if (tok && !strcmp(tok, "start"))
    {
      if (!el->band)
        el->band = elicit_band_new();

      elicit_band_show(el->band);
      el->state.shooting = 1;
    }
    else if (tok && !strcmp(tok, "stop"))
    {
      elicit_band_hide(el->band);
      el->state.shooting = 0;
    }
    else
      invalid = 1;
  }

  /* color selection */
  else if (!strcmp(tok, "pick")) 
  {
    tok = strtok(NULL, ",");
    if (tok && !strcmp(tok, "start"))
      el->state.picking = 1;
    else if (tok && !strcmp(tok, "stop"))
      el->state.picking = 0;
    else
      invalid = 1;
  }

  else if (!strcmp(tok, "scroll"))
  {
    tok = strtok(NULL, ",");
    if (tok && !strcmp(tok, "up"))
      elicit_scroll(el, source, 1);
    else if (tok && !strcmp(tok, "down"))
      elicit_scroll(el, source, -1);
  }

  else if (!strcmp(tok, "colorclass"))
  {
    int r,g,b,a;
    color_rgba_get(el->color, &r, &g, &b, &a);
    edje_object_color_class_set(el->obj.main, source, r, g, b, a, r, g, b, a, r, g, b, a);
  }

  /* quit */
  else if (!strcmp(tok, "quit"))
    ecore_main_loop_quit();

  /* unknown signal */
  else
    invalid = 1;

  if (invalid)
    fprintf(stderr, "[Elicit] Error: invalid signal: %s\n", emission);

  free(signal);
}

void
_elicit_cb_edje_move(void *data, Evas_Object *obj, const char *emission, const char *source)
{
  Elicit *el = data;


  if (el->state.shooting)
    elicit_shoot(el);

  if (el->state.picking)
    elicit_pick(el);
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
}

void
elicit_pick(Elicit *el)
{
  int x, y;
  int data;
  ecore_x_pointer_last_xy_get(&x, &y);
  if (elicit_grab_region(x, y, 1, 1, 0, &data))
    color_argb_int_set(el->color, data);
}

void
elicit_scroll(Elicit *el, const char *source, int dir)
{
}

static void
cb_color_changed(Color *color, void *data)
{
  int r,g,b;
  Elicit *el = data;
  int i;

  color_rgba_get(color, &r, &g, &b, NULL);

  if (el->obj.swatch)
    evas_object_color_set(el->obj.swatch, r, g, b, 255);

  for (i = 0; i < 3; i++)
  {
    if (el->obj.related[i])
    {
      float h,s,v;
      color_hsva_get(color, &h, &s, &v, NULL);
      h = ((int)h + related_color_offset[i]) % 360;
      evas_color_hsv_to_rgb(h,s,v,&r,&g,&b);
      evas_object_color_set(el->obj.related[i], r, g, b, 255);
    }
  }

  edje_object_part_text_set(el->obj.main, "elicit.color.hex", color_hex_get(el->color, COLOR_HEX_HASH | COLOR_HEX_CAPS));
}

Elicit *
elicit_new()
{
  Elicit *el;
  char buf[PATH_MAX];
  char *dir;
  
  el = calloc(sizeof(Elicit), 1);

  el->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 500, 500);
  el->evas = ecore_evas_get(el->ee);

  ecore_evas_title_set(el->ee, "Elicit");
  ecore_evas_name_class_set(el->ee, "Elicit", "Elicit");
  ecore_evas_borderless_set(el->ee, 1);
  ecore_evas_shaped_set(el->ee, 1);

  ecore_evas_data_set(el->ee, "Elicit", el);
  ecore_evas_callback_resize_set(el->ee, elicit_cb_ee_resize);
  ecore_evas_callback_mouse_in_set(el->ee, elicit_cb_ee_mouse_in);
  ecore_evas_callback_mouse_out_set(el->ee, elicit_cb_ee_mouse_out);

  el->obj.main = edje_object_add(el->evas);

  el->color = color_new();
  //XXX save and load this in config
  color_rgba_set(el->color, 255, 255, 255, 255);
  color_callback_changed_add(el->color, cb_color_changed, el);

  dir = br_find_data_dir(DATADIR);
  snprintf(buf, sizeof(buf), "%s/%s/", dir, PACKAGE);
  if (!ecore_file_exists(buf))
  {
    fprintf(stderr, "[Elicit] Warning: falling back to hardcoded data dir.\n");
    snprintf(buf, sizeof(buf), "%s/%s/", DATADIR, PACKAGE);
  }
  el->path.datadir = strdup(buf);
  free(dir);

  return el;
}

void
elicit_free(Elicit *el)
{
  if (el->color)
    color_unref(el->color);

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

  IF_FREE(el->conf.theme);
  IF_FREE(el->path.theme);
  IF_FREE(el->path.datadir);

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
elicit_theme_find(Elicit *el, const char *theme)
{
  static char buf[PATH_MAX];
  char *home;

  // check in home dir
  home = getenv("HOME");
  if (home)
  {
    snprintf(buf, sizeof(buf), "%s/.e/apps/elicit/themes/%s.edj", home, theme);
    if (ecore_file_exists(buf) && edje_file_group_exists(buf, "elicit.main"))
      return buf;
  }

  // check in sys dir
  snprintf(buf, sizeof(buf), "%s/themes/%s.edj", el->path.datadir, theme);
  if (ecore_file_exists(buf) && edje_file_group_exists(buf, "elicit.main"))
    return buf;


  return NULL;
}

void
elicit_theme_swallow_objs(Elicit *el)
{
  int i;

  /* magnified screen shot */
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
  else if (el->obj.shot)
  {
    evas_object_del(el->obj.shot);
    el->obj.shot = NULL;
  }

  /* main swatch */
  if (edje_object_part_exists(el->obj.main, "elicit.swatch"))
  {
    if (!el->obj.swatch)
      el->obj.swatch = evas_object_rectangle_add(el->evas);

    edje_object_part_swallow(el->obj.main, "elicit.swatch", el->obj.swatch);
  }
  else if (el->obj.swatch)
  {
    evas_object_del(el->obj.swatch);
    el->obj.swatch = NULL;
  }


  /* color slider */
  for (i = 0; i < 6; i++)
  {

    if (edje_object_part_exists(el->obj.main, cslider_part_names[i]))
    {
      el->obj.cslider[i] = elicit_cslider_add(el->evas);
      elicit_cslider_theme_set(el->obj.cslider[i], el->path.theme, "elicit.cslider");
      elicit_cslider_color_set(el->obj.cslider[i], el->color, i);
      edje_object_part_swallow(el->obj.main, cslider_part_names[i], el->obj.cslider[i]);
    }
    else if (el->obj.cslider[i])
    {
      evas_object_del(el->obj.cslider[i]);
      el->obj.cslider[i] = NULL;
    }
  }

  for (i = 0; i < 3; i++)
  {
    if (edje_object_part_exists(el->obj.main, related_color_names[i]))
    {
      if (!el->obj.related[i])
        el->obj.related[i] = evas_object_rectangle_add(el->evas);

      edje_object_part_swallow(el->obj.main, related_color_names[i], el->obj.related[i]);
    }
    else if (el->obj.related[i])
    {
      evas_object_del(el->obj.related[i]);
      el->obj.related[i] = NULL;
    }
  }
}

void
elicit_theme_unswallow_objs(Elicit *el)
{
  int i;

  if (!el->obj.main)
    return;

  if (el->obj.shot)
    edje_object_part_unswallow(el->obj.main, el->obj.shot);

  if (el->obj.swatch)
    edje_object_part_unswallow(el->obj.main, el->obj.swatch);

  for (i = 0; i < 6; i++)
    if (el->obj.cslider[i])
      edje_object_part_unswallow(el->obj.main, el->obj.cslider[i]);

  for (i = 0; i < 3; i++)
    if (el->obj.related)
      edje_object_part_unswallow(el->obj.main, el->obj.related[i]);
}

int
elicit_theme_set(Elicit *el, const char *theme)
{
  const char *path;
  int w, h;

  /* first check if full path is given */
  if (theme[0] == '/')
  {
    if (!edje_file_group_exists(theme, "elicit.main"))
      return 0;
  }
  else
    path = elicit_theme_find(el, theme);

  if (!path)
  {
    fprintf(stderr, "[Elicit] Error: theme \"%s\" not found.\n", theme);
    return 0;
  }

  if (el->path.theme) free(el->path.theme);
  el->path.theme = strdup(path);

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

  color_changed(el->color);

  return 1;
}

/**
 * Initialize libraries
 */
int
elicit_libs_init(void)
{
  BrInitError error;

  if (!br_init(&error) && error != BR_INIT_ERROR_DISABLED)
    fprintf(stderr, "[Elicit] Failed to initialize binreloc (error code: %d)\nFalling back to hardcoded paths.", error);

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


