#ifndef TILE_HH
#define TILE_HH

class Tile {
  
  bool _real;
  int _mark_val;
  
  short _number;
  short _match;
  short _row;
  short _col;
  short _lev;
  
  short _coverage;
  short _blocked;
  
 public:
  
  Tile();
  Tile(short r, short c, short l);
  
  short number() const			{ return _number; }
  short match() const			{ return _match; }
  short row() const			{ return _row; }
  short col() const			{ return _col; }
  short lev() const			{ return _lev; }
  
  void assign(short no, short m)	{ _number = no; _match = m; }
  void reset(bool is_real);
  void reset_blockage();
  
  void make_real()			{ _real = true; }
  void make_unreal()			{ _real = false; }
  
  bool real() const			{ return _real; }
  bool open() const			{ return !_blocked && !_coverage; }
  bool obscured() const			{ return _coverage >= 4; }
  
  void set_level_blockage(bool b)	{ _blocked = b; }
  void block_upper()			{ _coverage++; }
  void unblock_upper()			{ _coverage--; }
  
  void mark()				{ _mark_val = 1; }
  void unmark()				{ _mark_val = 0; }
  bool marked() const			{ return _mark_val != 0; }
  
  void set_mark_val(int m)		{ _mark_val = m; }
  int mark_val() const			{ return _mark_val; }
  
};


inline void
Tile::reset_blockage()
{
  _mark_val = 0;
  _coverage = 0;
  _blocked = 0;
}

inline void
Tile::reset(bool is_real)
{
  _real = is_real;
  reset_blockage();
}

#endif
