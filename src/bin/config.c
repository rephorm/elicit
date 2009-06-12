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
  el->conf.zoom = 4;
  el->conf.show_grid = 1;
}

int
elicit_config_save(Elicit *el)
{
  // XXX save config to disk...
}

