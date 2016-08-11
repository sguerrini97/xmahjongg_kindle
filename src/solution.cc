#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "solution.hh"
#include "panel.hh"
#include "board.hh"
#include "tile.hh"

SolutionDisplay::SolutionDisplay(Game *game, Board *board)
  : _game(game), _board(board),
    _on(false), _pos(0), _alarm(this), _state(0), _time_scale(3)
{
}


/* State values:
   0 initial pause	0.2 s
   1 flash on		0.3 s
   2 flash off		0.2 s
   3 flash on		0.3 s
   4 remove		0.1 s -> state 1
*/

bool
SolutionDisplay::turn_on(Panel *panel)
{
  if (_game->solution().size()) {
    int moves = 0;
    while (_game->undo())
      moves++;
    _state = 0;
    _pos = 0;
    _on = true;
    alarm();
    _alarm.schedule(Moment::now() + delay(200000 + 25000*moves));
  } else {
    panel->bell();
    _on = false;
  }
  return _on;
}


void
SolutionDisplay::real_turn_off()
{
  if (_state == 2 || _state == 4) {
    // Turn off the current highlighted move if necessary
    Move cur_move = _game->solution()[_pos];
    _board->unlight(cur_move.m1);
    _board->unlight(cur_move.m2);
    _board->flush();
  }
  _alarm.kill();
  _on = false;
  _time_scale = 3;
}


Moment
SolutionDisplay::delay(long ival) const
{
  ival = ival * 3 / _time_scale;
  return Moment(0, ival);
}


void
SolutionDisplay::change_speed(bool faster)
{
  if (faster) {
    if (_time_scale < 100)
      _time_scale = 2 * _time_scale - 2;
  } else {
    if (_time_scale > 3)
      _time_scale = (_time_scale + 2) / 2;
  }
}


void
SolutionDisplay::alarm()
{
  if (!_on) return;

  Moment now = Moment::now();
  Move cur_move = _game->solution()[_pos];
  
  switch (_state) {
    
   case 0:
    _alarm.schedule(now + delay(200000));
    _state = 1;
    break;
    
   case 1:
   case 2:
   case 3:
     {
       bool light = _state % 2;
       _board->set_lit(cur_move.m1, light);
       _board->set_lit(cur_move.m2, light);
       _board->flush();
       _alarm.schedule(now + delay(light ? 300000 : 200000));
       _state++;
       break;
     }
    
   case 4:
    if (!cur_move.m1->open() || !cur_move.m2->open())
      fatal_error("solution trying to remove non-free tiles!\n\
  This is a bug in xmahjongg. Please send mail to eddietwo@mit.edu telling\n\
  him so. Include the board number, which is %lu; the board\n\
  layout (e.g., \"default\" or \"bridge\"); and xmahjongg's\n\
  version number, which you get by running `xmahjongg --version'.",
		  (unsigned long)_game->board_number());
    _game->move(cur_move.m1, cur_move.m2);
    _pos++;
    if (_pos >= _game->solution().size()) {
      _on = false;
      _time_scale = 3;
    } else {
      _alarm.schedule(now + delay(100000));
      _state = 1;
    }
    break;
    
  }
}
