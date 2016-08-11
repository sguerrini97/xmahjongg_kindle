#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "kdets.hh"
#include "tile.hh"
#include "game.hh"
#include "swgeneral.hh"
#include <cstring>

#define NPICTURES	(Tileset::IVORY_NPICTURES)
#define NIMAGES		(2*NPICTURES)

#define NACROSS		9
#define NDOWN		5

#define BG_WIDTH	40
#define BG_HEIGHT	56
#define FACE_WIDTH	34
#define FACE_HEIGHT	50
#define FACE_XOFF	5
#define FACE_YOFF	1

#define NORMAL_BG_MAPPING	43
#define HIGHLIGHT_BG_MAPPING	44

KDETileset::KDETileset(Gif_Stream *gfs, Gif_XContext *gifx)
  : Tileset("ivory"), _gfs(gfs),
    _gifx(gifx), _colormap(0), _background_colormap(0),
    _image_error(ieNone),
    _background(None), _background_mask(None),
    _h_background(None), _h_background_mask(None)
{
  initialize_images();
  if (check()) {
    initialize();
    _gfs->refcount++;
  } else
    _gfs = 0;
}

KDETileset::~KDETileset()
{
  Gif_DeleteColormap(_colormap);
  Gif_DeleteColormap(_background_colormap);
  
  Display *display = _gifx->display;
  if (_gfs && _images.size())
    for (int i = 0; i < NIMAGES; i++) {
      if (_images[i]) XFreePixmap(display, _images[i]);
    }
  if (_background) XFreePixmap(display, _background);
  if (_background_mask) XFreePixmap(display, _background_mask);
  if (_h_background) XFreePixmap(display, _h_background);
  if (_h_background_mask) XFreePixmap(display, _h_background_mask);
  
  Gif_DeleteStream(_gfs);
}

void
KDETileset::initialize_images()
{
  _images.assign(NIMAGES, None);
  check_images();
}

void
KDETileset::check_images()
{
  if (_gfs == 0 || Gif_ImageCount(_gfs) != 1) {
    _image_error = ieBadGif;
    return;
  }

  Gif_Image *gfi = Gif_GetImage(_gfs, 0);
  if (!_gfs->global && !gfi->local) {
    _image_error = ieBadGif;
    return;
  }
  if (gfi->width != 360 || gfi->height != 280) {
    _image_error = ieBadSize;
    return;
  }

  _image_error = ieNone;
}

bool
KDETileset::check() const
{
  return _image_error == ieNone;
}

Pixmap
KDETileset::get_tile(int which, Pixmap *mask, bool big)
{
  Gif_Image *gfi = Gif_GetImage(_gfs, 0);
  Gif_UncompressImage(gfi);

  int left = (which % NACROSS) * BG_WIDTH + (big ? 0 : FACE_XOFF);
  int top = (which / NACROSS) * BG_HEIGHT + (big ? 0 : FACE_YOFF);
  int width = (big ? BG_WIDTH : FACE_WIDTH);
  int height = (big ? BG_HEIGHT : FACE_HEIGHT);

  bool restore_trans = gfi->transparent < 0;
  if (restore_trans)
    gfi->transparent = gfi->img[top][left];

  Pixmap img = Gif_XSubImage(_gifx, _gfs, gfi, left, top, width, height);
  *mask = Gif_XSubMask(_gifx, gfi, left, top, width, height);

  if (restore_trans)
    gfi->transparent = -1;
  return img;
}

void
KDETileset::initialize()
{
  Gif_Image *gfi = Gif_GetImage(_gfs, 0);
  _colormap = Gif_CopyColormap(gfi->local ? gfi->local : _gfs->global);

  _xborder = BG_WIDTH - FACE_WIDTH - 2;
  _yborder = BG_HEIGHT - FACE_HEIGHT - 2;
  _shadow = 1;
  
  _width = FACE_WIDTH + 2;
  _height = FACE_HEIGHT + 2;

  // create `_background' and `_h_background'
  _background = get_tile(NORMAL_BG_MAPPING, &_background_mask, true);
  _h_background = get_tile(HIGHLIGHT_BG_MAPPING, &_h_background_mask, true);
}


static int mapping[] = {
  27, 28, 29, 30,			/* 0-3: seasons */
  39, 40, 41, 42,			/* 4-7: plants; switch w/seasons?? */
  36, 37, 28,				/* 8-10: dragons */
  31, 32, 33, 34,			/* 11-14: winds */
  9, 10, 11, 12, 13, 14, 15, 16, 17,	/* 15-23: circles */
  18, 19, 20, 21, 22, 23, 24, 25, 26,	/* 24-32: bamboo */
  0, 1, 2, 3, 4, 5, 6, 7, 8,		/* 33-41: characters */
};

void
KDETileset::draw(SwDrawable *drawable, short x, short y, short which)
{
  if (!_images[which]) {
    // create new pixmap
    Pixmap pix = XCreatePixmap(_gifx->display, _gifx->drawable,
			       BG_WIDTH, BG_HEIGHT, _gifx->depth);
    if (!pix)
      return;

    // copy background onto pixmap
    XCopyArea(_gifx->display,
	      (which >= NPICTURES ? _h_background : _background), pix,
	      _gifx->image_gc, 0, 0, BG_WIDTH, BG_HEIGHT, 0, 0);

    // create subimage and copy it onto background
    Pixmap img, mask;
    int pos = (which >= NPICTURES ? which - NPICTURES : which);
    img = get_tile(mapping[pos], &mask);
    XSetClipMask(_gifx->display, _gifx->image_gc, mask);
    XSetClipOrigin(_gifx->display, _gifx->image_gc, FACE_XOFF, FACE_YOFF);
    XCopyArea(_gifx->display, img, pix, _gifx->image_gc,
	      0, 0, _width, _height, FACE_XOFF, FACE_YOFF);
    XSetClipMask(_gifx->display, _gifx->image_gc, None);
    XFreePixmap(_gifx->display, img);
    XFreePixmap(_gifx->display, mask);

    _images[which] = pix;
  }

  Pixmap mask = (which >= NPICTURES ? _h_background_mask : _background_mask);
  drawable->draw_image(_images[which], mask,
		       BG_WIDTH, BG_HEIGHT, x, y);
}

void
KDETileset::draw_normal(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, which);
}

void
KDETileset::draw_lit(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, which + NPICTURES);
}

void
KDETileset::draw_obscured(const Tile *, SwDrawable *drawable,
			  short x, short y)
{
  drawable->draw_image(_background, _background_mask,
		       _width + _xborder, _height + _yborder, x, y);
}
