#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "matches.hh"

MatchCount::MatchCount(SwWindow *window, Gif_Stream *gfs, const char *name)
  : SwWidget(window),
    _game(0), _one_image(None), _one_mask(None), _count(0)
{
  Gif_Image *gfi = Gif_GetNamedImage(gfs, name);
  if (gfi) {
    Gif_XContext *gifx = get_gif_x_context();
    _one_image = Gif_XImage(gifx, gfs, gfi);
    _one_mask = Gif_XMask(gifx, gfs, gfi);
    _one_width = Gif_ImageWidth(gfi);
    _one_height = Gif_ImageHeight(gfi);
  }
}

void
MatchCount::set_game(Game *g)
{
  assert(!_game);
  _game = g;
  _game->add_hook(this);
}

void
MatchCount::draw(int new_count, bool fast_display)
{
  if (!fast_display)
    _count = 0;
  
  int side = width() / _one_width;
  if (side == 0) side = 1;
  int level = (_count ? (_count - 1)/side : 0);
  int new_level = (new_count ? (new_count - 1)/side : 0);
  if (level > new_level) {
    clear_area(0, (new_level + 1) * _one_height,
	       width(), (level - new_level) * _one_height);
    _count = (new_level + 1) * side;
    level = new_level;
  }
  
  if (_count > new_count) {
    int pos = new_count % side;
    clear_area(pos * _one_width, level * _one_height,
	       (_count - new_count) * _one_width, _one_height);
  } else {
    int pos = _count % side;
    int y = (_count/side) * _one_height;
    for (int i = _count; i < new_count; i++) {
      int x = pos * _one_width;
      draw_image(_one_image, _one_mask, _one_width, _one_height, x, y);
      if (++pos >= side) {
	pos = 0;
	y += _one_height;
      }
    }
  }
  
  _count = new_count;
}

void
MatchCount::start_hook(Game *g)
{
  change(g->possible_moves());
}

void
MatchCount::move_made_hook(Game *g)
{
  change(g->possible_moves());
}
