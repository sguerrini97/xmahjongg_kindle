#ifndef SOLVABLE_HH
#define SOLVABLE_HH
#include "game.hh"

class SolvableMaker {
  
  Game *_game;
  Tileset *_tileset;
  Tile *_null_tile;
  const Vector<Tile *> &_tiles;
  
  Vector<Move> _solution;

  Tile *grid(int r, int c, int l) const	{ return _game->grid(r, c, l); }
  void print_level(int) const;
  
  void block_row(Tile *, bool);
  int blank_distance(Tile *, bool, bool = true) const;
  void unblock_tile(Tile *);
  void unblock_sides(Tile *);
  void choose(Tile *);
  void init_tiles();

  Tile *pick(int);
  bool try_assign(int *);
  
 public:
  
  SolvableMaker(Game *);
  
  bool assign(uint32_t);
  const Vector<Move> &solution() const		{ return _solution; }
  
};

#endif
