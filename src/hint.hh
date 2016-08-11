#ifndef HINT_HH
#define HINT_HH
#include "alarm.hh"
#include <lcdf/vector.hh>
#include "game.hh"
class Board;
class Tile;

class Hint: public AlarmHooks, public GameHooks {
  
  Board *_board;
  Game *_game;
  bool _on;
  
  int _which;
  int _skip_which;
  bool _state;
  Alarm _alarm;
  
  Vector<Tile *> _tiles;
  
  int _nchoices;
  int *_choices;
  
  static Moment flash_on_delay;
  static Moment flash_off_delay;
  
  void show(bool state);
  
  void get_choices();
  bool get_tiles(int);
  
 public:
  
  Hint(Board *);
  ~Hint();
  
  bool on() const				{ return _on; }

  bool find(int match);
  void next();
  void turn_off();
  
  void alarm();
  
  const Vector<Tile *> &tiles() const		{ return _tiles; }
  int ntiles() const				{ return _tiles.size(); }

  void start_hook(Game *);
  void add_tile_hook(Game *, Tile *);
  void remove_tile_hook(Game *, Tile *);
  
};

#endif
