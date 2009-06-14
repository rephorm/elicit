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
}

int
elicit_config_save(Elicit *el)
{
  // XXX save config to disk...
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

