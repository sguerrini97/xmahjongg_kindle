#ifndef SOLUTION_HH
#define SOLUTION_HH
#include "alarm.hh"
class Game;
class Board;
class Panel;

class SolutionDisplay: public AlarmHooks {
  
  Game *_game;
  Board *_board;
  
  bool _on;
  int _pos;
  Alarm _alarm;
  int _state;
  int _time_scale;

  void real_turn_off();

  Moment delay(long) const;
  
 public:
  
  SolutionDisplay(Game *, Board *);
  
  bool on() const			{ return _on; }
  
  bool turn_on(Panel *);
  void turn_off()			{ if (_on) real_turn_off(); }
  void change_speed(bool faster);
  
  void alarm();
  
};

#endif
