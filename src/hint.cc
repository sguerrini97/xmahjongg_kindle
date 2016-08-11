#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "hint.hh"
#include "game.hh"
#include "board.hh"
#include "tile.hh"

Moment Hint::flash_on_delay(0, 720000);
Moment Hint::flash_off_delay(0, 400000);


Hint::Hint(Board *b)
  : _board(b), _game(b->game()), _on(0),
    _which(0), _state(false), _alarm(this),
    _nchoices(0), _choices( new int[ _game->nmatches() ] )
{
  _game->add_hook(this);
}


Hint::~Hint()
{
  turn_off();
  delete[] _choices;
}


void
Hint::show(bool state)
{
  if (!_on || _state == state) return;
  
  _state = state;
  for (int i = 0; i < _tiles.size(); i++)
    if (!_board->tile_flag(_tiles[i], Board::fKeepLit))
      _board->set_lit(_tiles[i], state);
  _board->flush();
  
  Moment when = Moment::now();
  if (state)
    when += flash_on_delay;
  else
    when += flash_off_delay;
  _alarm.schedule(when);
}


void
Hint::get_choices()
{
  int i;
  int max_match = _game->nmatches();
  
  _nchoices = 0;
  for (i = 0; i < max_match; i++)
    if (_game->free_count(i) >= 2) {
      _choices[ _nchoices ] = i;
      _nchoices++;
    }
}


bool
Hint::get_tiles(int which)
{
  _tiles.clear();
  
  if (which >= 0 && which < _nchoices) {
    int tileclass = _choices[which];
    const Vector<Tile *> &t = _game->tiles();
    
    for (int i = 0; i < t.size(); i++)
      if (t[i]->real() && t[i]->open() && t[i]->match() == tileclass)
	_tiles.push_back( t[i] );
  }
  
  return _tiles.size() != 0;
}


bool
Hint::find(int match)
{
  if (!_on) {
    get_choices();
    _on = true;
  } else {
    show(false);
    _board->deselect();
  }
  
  _which = -1;
  for (int i = 0; i < _nchoices; i++)
    if (_choices[i] == match) {
      _skip_which = i;
      get_tiles(i);
      show(true);
      return true;
    }
  
  turn_off();
  return false;
}


void
Hint::next()
{
  if (_board->selected() && !_on) {
    if (!find( _board->selected()->match() )) {
      _board->bell();
      _board->deselect();
    }
    return;
  }
  
  if (!_on) {
    get_choices();
    _on = true;
  } else {
    show(false);
    _which++;
    if (_which == _skip_which) _which++;
    _board->deselect();
  }
  
  get_tiles(_which);
  
  if (_tiles.size())
    show(true);
  else {
    turn_off();
    _board->bell();
  }
}


void
Hint::turn_off()
{
  if (_on) show(false);
  _on = false;
  _which = 0;
  _skip_which = -1;
  _tiles.clear();
}


void
Hint::alarm()
{
  show(!_state);
}


void
Hint::start_hook(Game *)
{
  if (_on) turn_off();
}

void
Hint::add_tile_hook(Game *, Tile *)
{
  if (_on) turn_off();
}

void
Hint::remove_tile_hook(Game *, Tile *)
{
  if (_on) turn_off();
}
  
