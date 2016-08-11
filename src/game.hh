#ifndef GAME_HH
#define GAME_HH
#include <lcdf/vector.hh>
#include <lcdf/inttypes.h>
#include <stdio.h>
class Tile;
class Tileset;
class Game;

/* must have TILE_ROWS even */
#define TILE_ROWS 24
#define	TILE_COLS 38
#define TILE_LEVS 12

struct Move {
  
  Tile *m1;
  Tile *m2;
  
  Move()				{ }
  Move(Tile *t1, Tile *t2)		: m1(t1), m2(t2) { }
  
};


class GameHooks {
  
 public:

  virtual ~GameHooks()					{ }
  
  virtual void layout_hook(Game *)			{ }
  virtual void start_hook(Game *)			{ }
  virtual void add_tile_hook(Game *, Tile *)		{ }
  virtual void remove_tile_hook(Game *, Tile *)		{ }
  virtual void move_made_hook(Game *)			{ }
  
};


class Game {

  Tileset *_tileset;
  short _nmatches;
  
  Vector<Tile *> _tiles;
    
  uint32_t _board_number;
  short _taken;
  short _left;
  short _possible_moves;
  
  Vector<Move> _moves;
  Vector<short> _user_moves;
  int _user_move_pos;
  
  Vector<Move> _solution;
  
  Vector<GameHooks *> _hooks;
  
  Tile **_grid;
  
  bool _bad_free_count;
  int *_free_count;
  
  int *_left_count;
  
  bool init_grid();
  void init_blockage();
  
  void assign(uint32_t);
  void assign_solvable(uint32_t);
  
  void clear_layout();
  bool place_tile(int, int, int);
  void check_level_blockage(int, int, int) const;
  
  void wipe_info();
  void make_free_count();
  void count_possible_moves();
  
  bool layout_young_file(FILE *);
  bool layout_kyodai_file(FILE *);
  bool layout_kmahjongg_file(FILE *);
  
  static Tile the_null_tile;
  
 public:
  
  Game(Tileset *);
  ~Game();
  
  void add_hook(GameHooks *gh)			{ _hooks.push_back(gh); }
  void remove_hook(GameHooks *gh);
  
  int ntiles() const				{ return _tiles.size(); }
  Tile *tile(int i) const			{ return _tiles[i]; }
  const Vector<Tile *> &tiles() const		{ return _tiles; }
  
  Tile *&grid(short r, short c, short l) const;
  Tile *null_tile() const			{ return &the_null_tile; }

  Tileset *tileset() const			{ return _tileset; }
  uint32_t board_number() const		{ return _board_number; }
  
  void layout_default();
  int layout_file(const char *);
  void relayout();
  
  void start(uint32_t, bool);
  void start_specific(uint32_t);
  
  short nmatches() const			{ return _nmatches; }
  short nremaining() const			{ return _left; }
  int free_count(int);
  int left_count(int) const;
  int possible_moves();
  const Vector<Move> &solution() const		{ return _solution; }
  
  void add(Tile *);
  void remove(Tile *);
  
  void move(Tile *, Tile *, bool was_user = true);
  void mark_user_move();
  bool undo();
  bool redo();

  static uint32_t seed_to_board_number(uint32_t, bool);
  static void board_number_to_seed(uint32_t, uint32_t &, bool &);
  
};

extern void zrand_seed(uint32_t);
extern uint32_t zrand();


inline Tile *&
Game::grid(short r, short c, short l) const
{
  assert(r>=0 && r<TILE_ROWS && c>=0 && c<TILE_COLS && l>=0 && l<TILE_LEVS);
  return _grid[ (r * TILE_LEVS + l) * TILE_COLS + c ];
}

inline int
Game::free_count(int i)
{
  assert(i >= 0 && i < nmatches());
  if (_bad_free_count) make_free_count();
  return _free_count[i];
}

inline int
Game::left_count(int i) const
{
  assert(i >= 0 && i < nmatches());
  return _left_count[i];
}

inline int
Game::possible_moves()
{
  if (_possible_moves < 0) count_possible_moves();
  return _possible_moves;
}

extern void fatal_error(const char *, ...);
extern void error(const char *, ...);
extern void warning(const char *, ...);

#endif
