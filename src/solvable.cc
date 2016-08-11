#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "solvable.hh"
#include "tileset.hh"
#include "tile.hh"
#include <cstdio>

#define INVALID(t)		(t->mark_val() & 1)
#define TOUCHED(t)		(t->mark_val() & ~1)
#define VALIDATE(t)		(t->set_mark_val(TOUCHED(t)))
#define INVALIDATE(t)		(t->set_mark_val(3))


SolvableMaker::SolvableMaker(Game *game)
  : _game(game), _tileset(game->tileset()),
    _null_tile(game->null_tile()), _tiles(game->tiles())
{
}


void
SolvableMaker::print_level(int ll) const
{
  _null_tile->reset(false);
  for (int r = 2; r < TILE_ROWS - 2; r++) {
    for (int c = 0; c < TILE_COLS; c++) {
      Tile *t = grid(r, c, ll);
      if (r > 0 && grid(r-1, c, ll) == t)
	fprintf(stderr, t != _null_tile ? "^^^ " : "    ");
      else if (t->real())
	fprintf(stderr, "%c%c%c ", TOUCHED(t) ? '*' : '-',
		INVALID(t) ? '-' : 'O', !t->open() ? 'X' : '-');
      else if (t != _null_tile)
	fprintf(stderr, "::: ");
      else
	fprintf(stderr, "    ");
      c++;
    }
    fprintf(stderr, "\n");
  }
}


void
SolvableMaker::block_row(Tile *t, bool move_left)
{
  int rr = t->row();
  int cc = move_left ? t->col()-1 : t->col()+2;
  int ll = t->lev();
  
  for (int r = rr; r < rr + 2; r++) {
    Tile *side = grid(r, cc, ll);
    if (side->real() && !INVALID(side)) {
      INVALIDATE(side);
      block_row(side, move_left);
    }
  }
}


int
SolvableMaker::blank_distance(Tile *t, bool move_left, bool first) const
{
  if (!t->real())
    return t == _null_tile ? -1 : 0;
  
  int rr = t->row();
  int cc = move_left ? t->col()-1 : t->col()+2;
  int ll = t->lev();

  Tile *side = grid(rr, cc, ll);
  int d = blank_distance(side, move_left, false);
  if (side->row() != rr) {
    int d2 = blank_distance(grid(rr+1, cc, ll), move_left, false);
    if (d2 > d) d = d2;
  }
  
  if (first)
    return d;
  else
    return d < 0 ? d : 1 + d;
}


void
SolvableMaker::unblock_tile(Tile *t)
{
  // We can unblock a tile only if, on both sides, it immediately adjoins
  // a used tile -- or there's no used tile accessible.
  if (INVALID(t))
    if (blank_distance(t, true) <= 0
	&& blank_distance(t, false) <= 0)
      VALIDATE(t);
}


void
SolvableMaker::unblock_sides(Tile *t)
{
  int rr = t->row(), cc = t->col(), ll = t->lev();
  
  Tile *side = grid(rr, cc-1, ll);
  unblock_tile(side);
  if (side->row() != rr)
    unblock_tile(grid(rr+1, cc-1, ll));
  
  side = grid(rr, cc+2, ll);
  unblock_tile(side);
  if (side->row() != rr)
    unblock_tile(grid(rr+1, cc+2, ll));
}


void
SolvableMaker::choose(Tile *t)
{
  bool started_new_row = !TOUCHED(t);
  t->reset(false);
  
  // Mark all tiles reachable to the left and right.
  block_row(t, true);
  block_row(t, false);
  
  if (started_new_row)
    // Unmark tiles at the immediate left and right ONLY if we're starting a
    // new row.
    unblock_sides(t);
}


Tile *
SolvableMaker::pick(int tiles_done)
{
  int ntiles = _tiles.size();
  int offset = zrand() % (ntiles - tiles_done);
  int original_offset = offset;
  
  // First, choose a tile. We choose the `offset'th real open tile.
  while (1) {
    // Look for the tile.
    for (int tnum = 0; tnum < ntiles; tnum++) {
      Tile *t = _tiles[tnum];
      if (t->real() && t->open() && !INVALID(t)) {
	if (offset == 0) {
	  choose(t);
	  return t;
	}
	offset--;
      }
    }
    
    // If offset wasn't decremented, there are no available tiles; fail.
    if (offset == original_offset)
      return 0;
    
    // Reduce offset to be between 0 and (# available tiles - 1).
    offset = zrand() % (original_offset - offset);
  }
}


void
SolvableMaker::init_tiles()
{
  int ntiles = _tiles.size();
  
  // Clear all tile blockages
  for (int i = 0; i < ntiles; i++)
    _tiles[i]->reset(true);
  
  // Block tiles above
  for (int i = 0; i < ntiles; i++) {
    int rr = _tiles[i]->row();
    int cc = _tiles[i]->col();
    int ll = _tiles[i]->lev();
    for (int r = rr; r < rr + 2; r++) {
      grid(r, cc, ll+1)->block_upper();
      grid(r, cc+1, ll+1)->block_upper();
    }
  }
  
  // Make sure the null tile is set up properly! It cannot be marked, since
  // above we assume that all non-real tiles are unmarked.
  _null_tile->reset(false);
}


bool
SolvableMaker::try_assign(int *permuted_tiles)
{
  int ntiles = _tiles.size();
  init_tiles();
  
  // Now, pick tiles in pairs, and build up the solution in reverse order
  _solution.clear();
  for (int i = 0; i < ntiles / 2; i++) {
    Tile *t1 = pick(i * 2);
    Tile *t2 = pick(i * 2 + 1);
    if (!t1 || !t2) return false;
    
    _solution.push_back(Move(t1, t2));
    int permuted_val = permuted_tiles[i] * 2;
    t1->assign(permuted_val, _tileset->match(permuted_val));
    t2->assign(permuted_val + 1, _tileset->match(permuted_val));
    
    // Unblock tiles above and to the sides
    unblock_sides(t1);
    unblock_sides(t2);
    for (int k = 0; k < 2; k++) {
      Tile *t = k ? t1 : t2;
      int rr = t->row(), cc = t->col(), ll = t->lev();
      for (int r = rr; r < rr + 2; r++) {
	grid(r, cc, ll+1)->unblock_upper();
	grid(r, cc+1, ll+1)->unblock_upper();
      }
    }
  }
  
  return true;
}


bool
SolvableMaker::assign(uint32_t seed)
{
  /* assign creates a board which is guaranteed to be solvable. I
     thought for a long time about how to do this but impotently! I was
     thinking you wanted a function which, given a board, would return true
     (solvable) or false (not solvable). While this would still be nice for
     early warnings -- as soon as you fuck up and lose the game, xmahjongg
     would warn you -- it's a pretty hard problem. CREATING a solvable board
     is much easier; it's the reverse of solving the board: just drop down
     tiles two at a time! */
  
  int ntiles = _tiles.size();
  int *permuted_tiles = new int[ntiles / 2];
  
  // Decide on assignment order
  for (int i = 0; i < ntiles / 2; i++)
    permuted_tiles[i] = i;
  
  seed &= 0x3FFFFFFFU;
  zrand_seed(seed);
  for (int i = ntiles / 2 - 1; i > 0; i--) {
    int j = zrand() % (i+1);
    int tmp = permuted_tiles[i];
    permuted_tiles[i] = permuted_tiles[j];
    permuted_tiles[j] = tmp;
  }

  int retries = 0;
  while (!try_assign(permuted_tiles))
    retries++;
  //fprintf(stderr, "RRRRe %d\n", retries);
  
  // Reverse the solution
  Vector<Move> reversed_solution = _solution;
  _solution.clear();
  for (int i = reversed_solution.size() - 1; i >= 0; i--)
    _solution.push_back(reversed_solution[i]);
  
  delete[] permuted_tiles;
  return true;
}
