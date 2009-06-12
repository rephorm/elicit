#include "elicit.h"
#include "config.h"

Elicit *
elicit_new()
{
  Elicit *el;
  
  el = calloc(sizeof(Elicit), 1);

  el->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 100, 100);
  el->evas = ecore_evas_get(el->ee);

  ecore_evas_title_set(el->ee, "Elicit");
  ecore_evas_name_class_set(el->ee, "Elicit", "Elicit");
  ecore_evas_borderless_set(el->ee, 1);
  ecore_evas_shaped_set(el->ee, 1);

  el->gui = edje_object_add(el->evas);

  el->color = color_new();

  return el;
}

void
elicit_free(Elicit *el)
{
  color_free(el->color);
  evas_object_del(el->gui);
  ecore_evas_free(el->ee);

  return;
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
  snprintf(buf, sizeof(buf), "/home/rephorm/.e/apps/elicit/themes/%s.edj", theme);
  if (ecore_file_exists(buf))
    return buf;
}

void
elicit_theme_swallow_objs(Elicit *el)
{
}

void
elicit_theme_unswallow_objs(Elicit *el)
{
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

  edje_object_file_set(el->gui, path, "elicit");

  edje_object_size_min_get(el->gui, &w, &h);
  if (w > el->conf.w) el->conf.w = w;
  if (h > el->conf.h) el->conf.h = h;
  ecore_evas_size_min_set(el->ee, el->conf.w, el->conf.h);
  ecore_evas_resize(el->ee, el->conf.w, el->conf.h);

  evas_object_move(el->gui, 0, 0);
  evas_object_resize(el->gui, el->conf.w, el->conf.h);
  evas_object_show(el->gui);

  elicit_theme_swallow_objs(el);

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


