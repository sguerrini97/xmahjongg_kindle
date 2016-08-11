#ifndef TRAVERSE_HH
#define TRAVERSE_HH
#include "alarm.hh"
#include "game.hh"
#include <lcdf/vector.hh>
class Board;
class Hint;

class Traversal: public AlarmHooks, public GameHooks {
  
  Board *_board;
  Game *_game;
  
  bool _on;
  bool _state;
  Tile *_cursor;
  
  Vector<Tile *> _hint_on;
  
  Vector<Tile *> _horizontal;
  Vector<Tile *> _vertical;
  int _vertical_pos;
  
  Alarm _alarm;
  
  static Moment flash_on_delay;
  static Moment flash_off_delay;

  void show(bool);
  void turn_on(int, int, int);
  void turn_on(Tile *);
  void clear();

  void create_horizontal();
  void create_vertical();
  
 public:
  
  Traversal(Board *);
  ~Traversal();
  
  void turn_off();
  
  Tile *cursor() const;
  void set_cursor(Tile *);
  void next_horiz(bool right);
  void next_vert(bool down);

  void with_hint(Hint *, bool);
  
  void alarm();
  
  void layout_hook(Game *);
  void start_hook(Game *);
  void add_tile_hook(Game *, Tile *);
  void remove_tile_hook(Game *, Tile *);
  
};


inline Tile *
Traversal::cursor() const
{
  return _on ? _cursor : 0;
}

#endif
