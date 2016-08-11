#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "game.hh"
#include "solvable.hh"
#include "tile.hh"
#include "tileset.hh"
#include <cstdio>
#include <cstring>
#include <cctype>

Tile Game::the_null_tile;


Game::Game(Tileset *ts)
  : _tileset(ts), _nmatches(ts->nmatches()),
    _taken(0), _left(0), _possible_moves(0),
    _user_move_pos(0),
    _grid(new Tile *[TILE_ROWS * TILE_COLS * TILE_LEVS]),
    _bad_free_count(1), _free_count(new int[_nmatches]),
    _left_count(new int[_nmatches])
{
  _user_moves.push_back(0);
}


Game::~Game()
{
  clear_layout();
  delete[] _grid;
  delete[] _free_count;
  delete[] _left_count;
}


void
Game::remove_hook(GameHooks *gh)
{
  for (int i = 0; i < _hooks.size(); i++)
    if (_hooks[i] == gh) {
      _hooks[i] = _hooks.back();
      _hooks.pop_back();
      return;
    }
}


inline void
Game::wipe_info()
{
  _bad_free_count = 1;
  _possible_moves = -1;
}


void
Game::check_level_blockage(int r, int c, int l) const
{
  Tile *t = grid(r, c, l);
  if (t->real()) {
    r = t->row();
    c = t->col();
    bool left_blocked = grid(r, c-1, l)->real() || grid(r+1, c-1, l)->real();
    bool right_blocked = grid(r, c+2, l)->real() || grid(r+1, c+2, l)->real();
    t->set_level_blockage(left_blocked && right_blocked);
  }
}

void
Game::init_blockage()
{
  // reset all tiles
  for (int i = 0; i < _tiles.size(); i++)
    _tiles[i]->reset_blockage();
  
  for (int i = 0; i < _tiles.size(); i++)
    if (_tiles[i]->real()) {
      Tile *t = _tiles[i];
      int rr = t->row();
      int cc = t->col();
      int ll = t->lev();
      
      // blockage from tiles above
      if (ll > 0) {
	for (int r = rr; r < rr + 2; r++)
	  for (int c = cc; c < cc + 2; c++)
	    grid(r, c, ll-1)->block_upper();
      }
      
      check_level_blockage(rr, cc, ll);
    }
  
  // clear null tile blockage just for kicks
  null_tile()->reset(false);
}


uint32_t
Game::seed_to_board_number(uint32_t seed, bool solvable)
{
  assert((seed & 0xC0000000U) == 0);
  return ((seed & 0x3FFF0000) << 1)
    | (solvable ? 0x10000 : 0)
    | (seed & 0xFFFF);
}

void
Game::board_number_to_seed(uint32_t board_number, uint32_t &seed,
			   bool &solvable)
{
  assert((board_number & 0x80000000U) == 0);
  solvable = (board_number & 0x10000) != 0;
  seed = ((board_number & 0x7FFE0000) >> 1)
    | (board_number & 0xFFFF);
}


void
Game::assign(uint32_t seed)
{
  int ntiles = _tiles.size();
  int *permuted_tiles = new int[ntiles];
  
  for (int i = 0; i < ntiles; i++)
    permuted_tiles[i] = i;
  
  seed &= 0x3FFFFFFFU;
  zrand_seed(seed);
  _board_number = seed_to_board_number(seed, false);
  fprintf(stderr, "board number %u\n", _board_number);
  for (int i = ntiles - 1; i > 0; i--) {
    int j = zrand() % (i+1);
    int tmp = permuted_tiles[i];
    permuted_tiles[i] = permuted_tiles[j];
    permuted_tiles[j] = tmp;
  }
  
  for (int i = 0; i < ntiles; i++)
    _tiles[i]->assign( permuted_tiles[i],
		       _tileset->match( permuted_tiles[i] ) );
  
  delete[] permuted_tiles;
}

void
Game::assign_solvable(uint32_t seed)
{
  seed &= 0x3FFFFFFFU;
  _board_number = seed_to_board_number(seed, true);
  fprintf(stderr, "board number %u\n", _board_number);
  
  SolvableMaker sm(this);
  sm.assign(seed);
  _solution = sm.solution();
}


void
Game::start(uint32_t the_seed, bool solvable)
{
  _taken = 0;
  _left = _tiles.size();
  _solution.clear();
  
  if (solvable)
    assign_solvable(the_seed);
  else
    assign(the_seed);

  // Make all the tiles real and initialize the blockage.
  for (int i = 0; i < _left; i++)
    _tiles[i]->make_real();
  init_blockage();
  
  for (int i = 0; i < _nmatches; i++)
    _left_count[i] = 0;
  for (int i = 0; i < _left; i++)
    _left_count[_tiles[i]->match()]++;
  
  _moves.clear();
  _user_moves.resize(1);
  _user_move_pos = 0;
  wipe_info();
  
  for (int i = 0; i < _hooks.size(); i++)
    _hooks[i]->start_hook(this);
}


void
Game::start_specific(uint32_t board_number)
{
  if (board_number & 0x80000000U)
    fatal_error("bad board number");
  uint32_t seed;
  bool solvable;
  board_number_to_seed(board_number, seed, solvable);
  start(seed, solvable);
}


void
Game::add(Tile *t)
{
  assert(!t->real());
  t->make_real();
  
  int rr = t->row(), cc = t->col(), ll = t->lev();
  for (int r = rr; r < rr + 2; r++) {
    if (ll > 0) {
      grid(r, cc, ll-1)->block_upper();
      grid(r, cc+1, ll-1)->block_upper();
    }
    check_level_blockage(r, cc-1, ll);
    check_level_blockage(r, cc+2, ll);
  }
  
  _left++;
  _taken--;
  _left_count[t->match()]++;
  wipe_info();

  for (int i = 0; i < _hooks.size(); i++)
    _hooks[i]->add_tile_hook(this, t);
}


void
Game::remove(Tile *t)
{
  assert(t->real());
  t->make_unreal();
  
  int rr = t->row(), cc = t->col(), ll = t->lev();
  for (int r = rr; r < rr + 2; r++) {
    if (ll > 0) {
      grid(r, cc, ll-1)->unblock_upper();
      grid(r, cc+1, ll-1)->unblock_upper();
    }
    check_level_blockage(r, cc-1, ll);
    check_level_blockage(r, cc+2, ll);
  }
  
  _left--;
  _taken++;
  _left_count[t->match()]--;
  wipe_info();
  
  for (int i = 0; i < _hooks.size(); i++)
    _hooks[i]->remove_tile_hook(this, t);
}


void
Game::mark_user_move()
{
  _user_moves.push_back(_moves.size());
  _user_move_pos++;
  
  for (int i = 0; i < _hooks.size(); i++)
    _hooks[i]->move_made_hook(this);
}

void
Game::move(Tile *t1, Tile *t2, bool was_user)
{
  if (_user_move_pos < _user_moves.size() - 1) {
    int move_pos = _user_moves[_user_move_pos];
    _moves.resize(move_pos);
    _user_moves.resize(_user_move_pos + 1);
  }
  _moves.push_back(Move(t1, t2));
  remove(t1);
  remove(t2);
  if (was_user) mark_user_move();
}

bool
Game::undo()
{
  if (_user_move_pos == 0) return false;
  
  int end_move = _user_moves[_user_move_pos] - 1;
  int start_move = _user_moves[_user_move_pos - 1];
  _user_move_pos--;
  
  for (int i = end_move; i >= start_move; i--) {
    // Add tiles in opposite order to removing them or blockage values will
    // get screwed up
    add(_moves[i].m2);
    add(_moves[i].m1);
  }

  for (int i = 0; i < _hooks.size(); i++)
    _hooks[i]->move_made_hook(this);  
  return true;
}

bool
Game::redo()
{
  if (_user_move_pos == _user_moves.size() - 1) return false;
  
  _user_move_pos++;
  int start_move = _user_moves[_user_move_pos - 1];
  int end_move = _user_moves[_user_move_pos] - 1;
  
  for (int i = start_move; i <= end_move; i++) {
    remove(_moves[i].m1);
    remove(_moves[i].m2);
  }
  
  for (int i = 0; i < _hooks.size(); i++)
    _hooks[i]->move_made_hook(this);
  return true;
}

void
Game::make_free_count()
{
  int i;
  int nmatch = _tileset->nmatches();
  
  for (i = 0; i < nmatch; i++)
    _free_count[i] = 0;
  
  for (i = 0; i < _tiles.size(); i++)
    if (_tiles[i]->real() && _tiles[i]->open()) {
      int mc = _tiles[i]->match();
      _free_count[mc]++;
    }
  
  _bad_free_count = false;
}

void
Game::count_possible_moves()
{
  int nmatch = nmatches();
  if (_bad_free_count) make_free_count();
  
  int i, j = 0;
  for (i = 0; i < nmatch; i++)
    if (_free_count[i] == 2) j += 1;
    else if (_free_count[i] == 3) j += 3;
    else if (_free_count[i] == 4) j += 6;
  
  _possible_moves = j;
}



void
Game::clear_layout()
{
  int i;
  for (i = 0; i < _tiles.size(); i++)
    delete _tiles[i];
  _tiles.clear();
}

static int
tile_sorter(const void *v1, const void *v2)
{
  Tile *t1 = *(Tile **)v1;
  Tile *t2 = *(Tile **)v2;
  if (t1->row() != t2->row())
    return t1->row() - t2->row();
  if (t1->col() != t2->col())
    return t1->col() - t2->col();
  return t2->lev() - t1->lev();
}

bool
Game::init_grid()
{
  // Clear the grid.
  for (int r = 0; r < TILE_ROWS; r++)
    for (int c = 0; c < TILE_COLS; c++)
      for (int l = 0; l < TILE_LEVS; l++)
	grid(r, c, l) = null_tile();

  int tile_count = _tiles.size();
  for (int i = 0; i < tile_count; i++) {
    Tile *t = _tiles[i];
    for (int r = t->row(); r < t->row() + 2; r++)
      for (int c = t->col(); c < t->col() + 2; c++) {
	// Ensure that any 2 tiles occupy disjoint space.
	if (grid(r, c, t->lev()) != null_tile()) {
	  fprintf(stderr, "tile overlaps existing tile at %d,%d,%d\n", r, c, t->lev());
	  return false;
	}
	grid(r, c, t->lev()) = t;
      }
  }
  
  // Sort the tiles in row, then column, then reverse-level order.
  qsort(&_tiles[0], tile_count, sizeof(Tile *), &tile_sorter);

  // Done
  return true;
}


bool
Game::place_tile(int r, int c, int l)
{
  if (r > 1 && c > 1 && l >= 0
      && r < TILE_ROWS - 3 && c < TILE_COLS - 3 && l < TILE_LEVS - 1) {
    _tiles.push_back(new Tile(r, c, l));
    return true;
    
  } else {
    fprintf(stderr, "tile out of bounds at %d,%d,%d\n", r, c, l);
    return false;
  }
}


void
Game::layout_default()
{
  clear_layout();
  
  int i, j;
  for (j = 2; j <= 13; j++) {
    place_tile(2, j*2, 0);
    place_tile(8, j*2, 0);
    place_tile(10, j*2, 0);
    place_tile(16, j*2, 0);
  }
  
  for (j = 3; j <= 12; j++) {
    place_tile(6, j*2, 0);
    place_tile(12, j*2, 0);
  }
  
  for (j = 4; j <= 11; j++) {
    place_tile(4, j*2, 0);
    place_tile(14, j*2, 0);
  }
  
  for (j = 5; j <= 10; j++)
    for (i = 2; i <= 7; i++)
      place_tile(i*2, j*2, 1);
  
  for (j = 6; j <= 9; j++)
    for (i = 3; i <= 6; i++)
      place_tile(i*2, j*2, 2);
  
  for (j = 7; j <= 8; j++)
    for (i = 4; i <= 5; i++)
      place_tile(i*2, j*2, 3);
  
  place_tile(9, 2, 0);
  place_tile(9, 28, 0);
  place_tile(9, 30, 0);
  place_tile(9, 15, 4);
  
  init_grid();
  for (j = 0; j < _hooks.size(); j++)
    _hooks[j]->layout_hook(this);
}

bool
Game::layout_kyodai_file(FILE *f)
{
  // skip `Kyodai 3.0'
  int c = getc(f);
  while (c != '\n' && c != EOF) c = getc(f);
  // skip board identifier
  c = getc(f);
  while (c != '\n' && c != EOF) c = getc(f);

  for (int lev = 0; lev < 5; lev++)
    for (int row = 0; row < 20; row++)
      for (int col = 0; col < 34; col++) {
	c = ' ';
	while (isspace(c)) c = getc(f);
	if (c == '0')
	  /* don't place a tile */;
	else if (c == '1') {
	  if (!place_tile(row + 2, col + 2, lev))
	    return false;
	} else {
	  fprintf(stderr, "bad character");
	  return false;
	}
      }
  
  return true;
}

bool
Game::layout_young_file(FILE *f)
{
  char buffer[BUFSIZ];
  while (!feof(f)) {
    buffer[0] = 0;
    fgets(buffer, BUFSIZ, f);
    int r, c, l;
    if (sscanf(buffer, " %d %d %d", &r, &c, &l) == 3)
      if (!place_tile(r+2, c+2, l))
	return false;
  }
  return true;
}

bool
Game::layout_kmahjongg_file(FILE *f)
{
  char buf[BUFSIZ];

  // check for `kmahjongg-layout'
  buf[0] = 0;
  fgets(buf, BUFSIZ, f);
  if (memcmp(buf, "kmahjongg-layout-v1", 19) != 0) {
    fprintf(stderr, "not a kmahjongg layout file\n");
    return false;
  }

  // now read file
  int l = 0;
  while (!feof(f) && l < TILE_LEVS - 1) {
    for (int r = 0; r < 16 && !feof(f); r++) {
      buf[0] = 0;
      fgets(buf, BUFSIZ, f);
      for (int c = 0; c < TILE_COLS - 3 && buf[c] && !isspace(buf[c]); c++)
	if (buf[c] == '1') {
	  if (!place_tile(r + 2, c + 2, l))
	    return false;
	} else if (buf[c] == '2' || buf[c] == '3' || buf[c] == '4'
		   || buf[c] == '.')
	  /* ok */;
	else if (buf[c] == '#')
	  break;
	else {
	  fprintf(stderr, "bad character\n");
	  return false;
	}
    }
    l++;
  }
  
  return true;
}

int
Game::layout_file(const char *filename)
  /* returns -1 on system error, 0 on other error, 1 on no error */
{
  clear_layout();
  
  FILE *f = fopen(filename, "r");
  if (!f) return -1;
  
  bool ok;
  int c = getc(f);
  ungetc(c, f);
  if (c == 'K')
    ok = layout_kyodai_file(f);
  else if (c == 'k')
    ok = layout_kmahjongg_file(f);
  else
    ok = layout_young_file(f);
  
  fclose(f);
  
  if (!ok)
    return 0;
  else if (_tiles.size() != _tileset->ntiles()) {
    fprintf(stderr, "%d tiles (should be %d)\n", _tiles.size(), _tileset->ntiles());
    return 0;
  } else if (!init_grid()) {
    fprintf(stderr, "some tiles occupied the same space\n");
    return 0;
  }
  
  for (int j = 0; j < _hooks.size(); j++)
    _hooks[j]->layout_hook(this);
  return 1;
}

void
Game::relayout()
{
  for (int j = 0; j < _hooks.size(); j++)
    _hooks[j]->layout_hook(this);
}
