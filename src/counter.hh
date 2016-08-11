#ifndef COUNTER_HH
#define COUNTER_HH
#include "swwidget.hh"
#include "game.hh"
class Panel;

class FancyCounter: public SwWidget {
  
  int _digits;
  int _value;
  
 public:
  
  FancyCounter(SwWindow *, int);
  
  void set_value(int v)				{ _value = v; draw(); }
  void draw();
  
};

class FancyTileCounter: public FancyCounter, public GameHooks {

 public:

  FancyTileCounter(SwWindow *);

  void start_hook(Game *);
  void move_made_hook(Game *);

};

#endif
