#ifndef GMJTS_HH
#define GMJTS_HH
#include <lcdfgif/gif.h>
#include <lcdfgif/gifx.h>
#include "tileset.hh"

class GnomeMjTileset: public Tileset {
  
  enum ImageError {
    ieNone = 0,
    ieBadGif,
    ieBadSize,
  };
  
  Gif_Stream *_gfs;
  Gif_XContext *_gifx;
  Gif_Colormap *_colormap;
  int _image_error;
  
  Vector<Pixmap> _images;
  Vector<Pixmap> _masks;
  
  void initialize_images();
  void check_images();
  bool check() const;
  void initialize();
  
  void draw(SwDrawable *, short, short, short);
  
 public:
  
  GnomeMjTileset(Gif_Stream *, Gif_XContext *);
  ~GnomeMjTileset();
  
  bool ok() const				{ return _gfs; }
  
  void draw_normal(const Tile *, SwDrawable *, short x, short y);
  void draw_lit(const Tile *, SwDrawable *, short x, short y);
  void draw_obscured(const Tile *, SwDrawable *, short x, short y);
  
};

#endif
