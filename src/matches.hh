#ifndef MATCHES_HH
#define MATCHES_HH
#include "swwidget.hh"
#include "game.hh"

class MatchCount: public SwWidget, public GameHooks {

  Game *_game;
  
  Pixmap _one_image;
  Pixmap _one_mask;
  int _one_width;
  int _one_height;

  int _count;
  
  void draw(int, bool fast_display);
  
 public:
  
  MatchCount(SwWindow *, Gif_Stream *, const char *);
  
  void set_game(Game *);
  
  void change(int m)				{ draw(m, true); }
  void draw()					{ draw(_count, false); }
  
  void start_hook(Game *);
  void move_made_hook(Game *);
  
};

#endif
