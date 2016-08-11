#ifndef KDETS_HH
#define KDETS_HH
#include <lcdfgif/gif.h>
#include <lcdfgif/gifx.h>
#include "tileset.hh"

class KDETileset: public Tileset {
  
  enum ImageError {
    ieNone = 0,
    ieBadGif,
    ieBadSize,
  };
  
  Gif_Stream *_gfs;
  Gif_XContext *_gifx;
  Gif_Colormap *_colormap;
  Gif_Colormap *_background_colormap;
  int _image_error;
  
  Vector<Pixmap> _images;
  Pixmap _background;
  Pixmap _background_mask;
  Pixmap _h_background;
  Pixmap _h_background_mask;
  
  void initialize_images();
  void check_images();
  bool check() const;
  void initialize();

  Pixmap get_tile(int, Pixmap *, bool = false);
  void draw(SwDrawable *, short, short, short);
  
 public:
  
  KDETileset(Gif_Stream *, Gif_XContext *);
  ~KDETileset();
  
  bool ok() const				{ return _gfs; }
  
  void draw_normal(const Tile *, SwDrawable *, short x, short y);
  void draw_lit(const Tile *, SwDrawable *, short x, short y);
  void draw_obscured(const Tile *, SwDrawable *, short x, short y);
  
};

#endif
