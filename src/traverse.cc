#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "traverse.hh"
#include "board.hh"
#include "tile.hh"
#include "hint.hh"
#include <cstdlib>

Moment Traversal::flash_on_delay(0, 400000);
Moment Traversal::flash_off_delay(0, 150000);


Traversal::Traversal(Board *b)
  : _board(b), _game(b->game()), _on(false),
    _cursor(0), _alarm(this)
{
  _game->add_hook(this);
  layout_hook(_game);
}

Traversal::~Traversal()
{
  _game->remove_hook(this);
}


void
Traversal::show(bool state)
{
  if (!_on || _state == state) return;
  _state = state;
  
  Tile *t = cursor();
  if (_state)
    _board->light(t);
  else if (_board->selected() != t)
    _board->unlight(t);
  _board->flush();
  
  Moment when = Moment::now();
  if (state)
    when += flash_on_delay;
  else
    when += flash_off_delay;
  _alarm.schedule(when);
}

void
Traversal::set_cursor(Tile *cursor)
{
  if (_on && _cursor != cursor) show(false);
  _cursor = cursor;
}

void
Traversal::turn_on(int r, int c, int l)
{
  if (_on) show(false);
  assert(r || c || l);
  _cursor = _game->grid(r, c, l);
  _on = true;
  show(true);
}


void
Traversal::turn_on(Tile *t)
{
  if (_on) show(false);
  assert(t);
  _cursor = t;
  _on = true;
  show(true);
}


void
Traversal::turn_off()
{
  if (_on) show(false);
  _on = false;
  for (int i = 0; i < _hint_on.size(); i++) {
    _board->set_tile_flag(_hint_on[i], Board::fKeepLit, false);
    _board->unlight(_hint_on[i]);
  }
}


void
Traversal::clear()
{
  _cursor = 0;
  _on = false;
  _hint_on.clear();
}


void
Traversal::create_horizontal()
{
  /* Horizontal traversal: What a massive pain in the ass traversal has been!
     Here's the problem. I seem to have an internal "algorithm" for what the
     traversal pattern should be, but it's based on information from the 2D
     board -- hard to code -- and even then, I'm not sure how to formalize
     it!! So the problem is to approximate this, using only relatively local
     information. To be "as right as possible" without ever being DEAD WRONG.
     (Being dead wrong some of the time is worse than being partially right
     all of the time, I assert from experience.)
     
     So here is the idea. Consider groups of 2 rows at a time, row r and row
     r + 1. Sort these together. The End.

     A slight improvement: Assume all boards are vertically symmetrical, and
     don't move indiscriminately over the axis of symmetry. This solves this
     problem:

        XX XX			If X and Y sort together, then Z and Q
      YYXX XXYY			should also sort together. But the presence
      YY     YYMM____axis	of M screwed this up: we had 3 groups --
      ZZ     ZZMM		X+Y, M+Z, Q.
      ZZQQ QQZZ
        QQ QQ			With axis consideration, this won't happen:
				we won't group M+Z because M is above the
				axis and Z is on it.
  */
  
  _horizontal.clear();
  const Vector<Tile *> &tiles = _game->tiles();
  int ntiles = _game->ntiles();
  if (ntiles == 0) return;
  
  Vector<short> row_off;
  int cur_row = -1;
  for (int i = 0; i < ntiles; i++)
    if (tiles[i]->row() != cur_row) {
      row_off.push_back(i);
      cur_row = tiles[i]->row();
    }
  row_off.push_back(ntiles);
  row_off.push_back(ntiles);
  
  int symmetry_axis = (tiles[ntiles - 1]->row() + tiles[0]->row()) / 2 + 1;
  
  for (int rowi = 0; rowi < row_off.size() - 2; /* incremented inside */) {
    int cur_row = tiles[row_off[rowi]]->row();
    
    while (row_off[rowi+1] < ntiles
	   && tiles[row_off[rowi+1]]->row() == cur_row + 1
	   && cur_row != symmetry_axis) {
      // Consider here two rows at a time.
      int r1 = row_off[rowi];
      int r2 = row_off[rowi+1];
      
      while (r1 < row_off[rowi+1] && r2 < row_off[rowi+2])
	if (tiles[r1]->col() < tiles[r2]->col()) {
	  _horizontal.push_back(tiles[r1]);
	  r1++;
	} else {
	  _horizontal.push_back(tiles[r2]);
	  r2++;
	}
      
      for (; r1 < row_off[rowi+1]; r1++)
	_horizontal.push_back(tiles[r1]);
      for (; r2 < row_off[rowi+2]; r2++)
	_horizontal.push_back(tiles[r2]);
      
      rowi += 2;
      cur_row += 2;
    }
    
    // There might be one row left over.
    if (row_off[rowi] < ntiles && tiles[row_off[rowi]]->row() == cur_row) {
      for (int r1 = row_off[rowi]; r1 < row_off[rowi+1]; r1++)
	_horizontal.push_back(tiles[r1]);
      rowi++;
      cur_row++;
    }
  }
}


void
Traversal::next_horiz(bool right)
{
  int horiz_count = _horizontal.size();
  _vertical_pos = -1;
  
  int i;
  if (!_cursor)
    i = right ? horiz_count - 1 : 0;
  else
    for (i = 0; i < horiz_count; i++)
      if (_horizontal[i] == _cursor)
	break;
  
  assert(i < horiz_count);
  int delta = right ? 1 : horiz_count - 1;
  for (int c = 0; c < horiz_count; c++) {
    i = (i + delta) % horiz_count;
    if (_horizontal[i]->open() && _horizontal[i]->real()) {
      turn_on(_horizontal[i]);
      return;
    }
  }
  turn_off();
}


void
Traversal::create_vertical()
{
  const Vector<Tile *> &tiles = _game->tiles();
  Vector<Tile *> list;
  
  for (int i = 0; i < tiles.size(); i++)
    if (tiles[i]->real() && tiles[i]->open())
      list.push_back(tiles[i]);
  if (_cursor && (!_cursor->real() || !_cursor->open()))
    list.push_back(_cursor);
  for (int i = 0; i < list.size(); i++)
    list[i]->unmark();
  
  _vertical.clear();
  _vertical_pos = -1;
  
  while (1) {
    
    // Find the leftmost unmarked tile.
    int leftmost = TILE_COLS + 1;
    for (int i = 0; i < list.size(); i++)
      if (!list[i]->marked() && list[i]->col() < leftmost)
	leftmost = list[i]->col();
    if (leftmost > TILE_COLS)
      // no leftmost tile; we're done
      break;

    int last_row = -1;
    for (int r = 0; r < TILE_ROWS; r++) {
      
      // Find the closest tile to the leftmost column in this row
      Tile *closest = 0;
      int closest_dist = TILE_COLS + 1;
      for (int i = 0; i < list.size(); i++)
	if (list[i]->row() == r) {
	  int dist = abs(list[i]->col() - leftmost);
	  if (list[i]->marked()) dist += 4;
	  if (r == last_row + 1) dist += 4;
	  if (dist < closest_dist) {
	    closest = list[i];
	    closest_dist = dist;
	  }
	}
      
      // Check to see that it is close enough, and that it's not a repeat
      if (_vertical.size() && _vertical.back() == closest)
	continue;
      
      if (closest_dist > 6)
	continue;
      
      _vertical.push_back(closest);
      closest->mark();
      last_row = r;
    }
  }
}


void
Traversal::next_vert(bool down)
{
  if (!_vertical.size())
    create_vertical();
  int vert_count = _vertical.size();
  
  if (_cursor && _vertical_pos < 0)
    for (int i = 0; i < vert_count && _vertical_pos < 0; i++)
      if (_vertical[i] == _cursor)
	_vertical_pos = i;

  int i = _vertical_pos;
  if (i < 0)
    i = down ? vert_count - 1 : 0;
  
  assert(i >= 0 && i < vert_count);
  int delta = down ? 1 : vert_count - 1;
  for (int c = 0; c < vert_count; c++) {
    i = (i + delta) % vert_count;
    if (_vertical[i]->open() && _vertical[i]->real()) {
      turn_on(_vertical[i]);
      _vertical_pos = i;
      return;
    }
  }
  turn_off();
}


void
Traversal::with_hint(Hint *hint, bool take)
{
  assert(!_on && hint->ntiles());
  
  const Vector<Tile *> &hints = hint->tiles();
  int nhints = hint->ntiles();
  int match = hints[0]->match();
  Tile *selected = _board->selected();
  
  if (nhints == 2 && (take || selected)) {
    _game->move(hints[0], hints[1]);
    
  } else if (nhints == 4 && (take || selected)) {
    _game->move(hints[0], hints[1]);
    hint->find(match);
    
  } else if (nhints == 2 || nhints == 4) {
    _board->select(hints[0]);
    
  } else if (!_hint_on.size()) {
    _hint_on.push_back(hints[0]);
    _hint_on.push_back(hints[1]);
    if (selected && hints[1] != selected) _hint_on[0] = selected;
    goto turn_on_hint;
    
  } else if (take) {
    assert(_hint_on.size() == 2);
    _game->move(_hint_on[0], _hint_on[1]);
    
  } else {
    int k;
    for (k = 0; k < 3; k++)
      if (hints[k] != _hint_on[0] && hints[k] != _hint_on[1])
	break;
    
    _hint_on[0] = hints[k];
    _hint_on[1] = hints[(k+1)%3];
    _board->set_tile_flag(hints[(k+2)%3], Board::fKeepLit, false);
    _board->unlight(hints[(k+2)%3]);
    
   turn_on_hint:
    _board->set_tile_flag(_hint_on[0], Board::fKeepLit, true);
    _board->set_tile_flag(_hint_on[1], Board::fKeepLit, true);
    _board->light(_hint_on[0]);
    _board->light(_hint_on[1]);
  }
}  


void
Traversal::alarm()
{
  show(!_state);
}


void
Traversal::layout_hook(Game *g)
{
  assert(g == _game);
  create_horizontal();
}

void
Traversal::start_hook(Game *)
{
  _vertical.clear();
  _cursor = 0;
  _hint_on.clear();
}

void
Traversal::add_tile_hook(Game *, Tile *)
{
  _vertical.clear();
  _hint_on.clear();
}

void
Traversal::remove_tile_hook(Game *, Tile *)
{
  _vertical.clear();
  _hint_on.clear();
}
