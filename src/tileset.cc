#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "tileset.hh"
#include <cstring>

Tileset::Tileset(short ntiles, short npictures, short nmatches)
  : _ntiles(ntiles), _npictures(npictures), _nmatches(nmatches),
    _pictures(ntiles, -1), _matches(ntiles, -1)
{
}

Tileset::Tileset(const char *name)
{
  if (strcmp(name, "ivory") == 0)
    initialize_ivory();
  else
    assert(0 && "bad Tileset name");
}


/*****
 * THE IVORY TILE SET ORGANIZATION
 *
 * There are 144 tiles.
 *
 * Tile pictures are as follows:
 * 0-3    seasons    (spring, summer, autumn, winter)
 * 4-7    plants     (bamboo, orchid, plum, chrysanthemum)
 * 8-10   dragons    (green, white, red)
 * 11-14  winds      (north, south, east, west)
 * 15-23  circles    (1-9)
 * 24-32  bamboo     (1-9)
 * 33-41  characters (1-9)
 *
 * Tile matches are as follows:
 * 0      seasons
 * 1      plants
 * 2-4    dragons    (green, white, red)
 * 5-8    winds      (north, south, east, west)
 * 9-17   circles    (1-9)
 * 18-26  bamboo     (1-9)
 * 27-35  characters (1-9)
 **/

void
Tileset::initialize_ivory()
{
  // Set up the _pictures and _matches arrays as required according to the
  // plan laid out above.
  _ntiles = IVORY_NTILES;
  _npictures = IVORY_NPICTURES;
  _nmatches = IVORY_NMATCHES;
  _pictures.assign(_ntiles, -1);
  _matches.assign(_ntiles, -1);
  
  // Seasons and flowers: one match per 4 tiles, one picture per tile.
  for (int t = 0; t < 8; t++) {
    _pictures[t] = t;
    _matches[t] = t / 4;
  }

  // Everything else: one match and one picture per 4 tiles.
  int cur_picture = 8;
  int cur_match = 2;
  for (int t = 8; t < _ntiles; t += 4) {
    for (int j = 0; j < 4; j++) {
      _pictures[t+j] = cur_picture;
      _matches[t+j] = cur_match;
    }
    cur_picture++;
    cur_match++;
  }
}

static const char *ivory_picture_names[] = {
  "season-1",	"season-2",	"season-3",	"season-4",	"flower-1",
  "flower-2",	"flower-3",	"flower-4",	"dragon-1",	"dragon-2",
  "dragon-3",	"wind-1",	"wind-2",	"wind-3",	"wind-4",
  "dot-1",	"dot-2",	"dot-3",	"dot-4",	"dot-5",
  "dot-6",	"dot-7",	"dot-8",	"dot-9",	"bamboo-1",
  "bamboo-2",	"bamboo-3",	"bamboo-4",	"bamboo-5",	"bamboo-6",
  "bamboo-7",	"bamboo-8",	"bamboo-9",	"character-1",	"character-2",
  "character-3","character-4",	"character-5",	"character-6",	"character-7",
  "character-8","character-9",
};

const char *
Tileset::ivory_picture_name(int p)
{
  assert(p >= 0 && p < IVORY_NPICTURES);
  return ivory_picture_names[p];
}
