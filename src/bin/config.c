#include "elicit.h"
#include "config.h"

int
elicit_config_load(Elicit *el)
{
  //XXX actually load this from disk...
  el->conf.x = 0;
  el->conf.y = 0;
  el->conf.w = 200;
  el->conf.h = 200;
  el->conf.zoom_level = 4;
  el->conf.grid_visible = 1;
  el->conf.show_band = 1;
  return 1;
}

int
elicit_config_save(Elicit *el)
{
  if (el->palette) palette_save(el->palette);
  // XXX save config to disk...
  return 0;
}

void
elicit_config_zoom_level_set(Elicit *el, int zoom_level)
{
  if (el->conf.zoom_level == zoom_level) return;

  el->conf.zoom_level = zoom_level;
  el->conf.changed = 1;
}

int
elicit_config_zoom_level_get(Elicit *el)
{
  return el->conf.zoom_level;
}

void
elicit_config_grid_visible_set(Elicit *el, int grid_visible)
{
  if (el->conf.grid_visible == grid_visible) return;

  el->conf.grid_visible = grid_visible;
  el->conf.changed = 1;
}

int
elicit_config_grid_visible_get(Elicit *el)
{
  return el->conf.grid_visible;
}

void
elicit_config_show_band_set(Elicit *el, int show_band)
{
  if (el->conf.show_band == show_band) return;

  el->conf.show_band = show_band;
  el->conf.changed = 1;
}

int
elicit_config_show_band_get(Elicit *el)
{
  return el->conf.show_band;
}
