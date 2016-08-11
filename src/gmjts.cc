#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "gmjts.hh"
#include "tile.hh"
#include "swgeneral.hh"
#include <cstring>

#define NPICTURES	(Tileset::IVORY_NPICTURES)
#define NIMAGES		(2*NPICTURES + 1)

#define NACROSS		21
#define NDOWN		4

GnomeMjTileset::GnomeMjTileset(Gif_Stream *gfs, Gif_XContext *gifx)
  : Tileset("ivory"), _gfs(gfs),
    _gifx(gifx), _colormap(0), _image_error(ieNone)
{
  initialize_images();
  if (check()) {
    initialize();
    _gfs->refcount++;
  } else
    _gfs = 0;
}

GnomeMjTileset::~GnomeMjTileset()
{
  Gif_DeleteColormap(_colormap);
  
  Display *display = _gifx->display;
  if (_gfs && _images.size())
    for (int i = 0; i < NIMAGES; i++) {
      if (_images[i]) XFreePixmap(display, _images[i]);
      if (_masks[i]) XFreePixmap(display, _masks[i]);
    }
  
  Gif_DeleteStream(_gfs);
}

void
GnomeMjTileset::initialize_images()
{
  _images.assign(NIMAGES, None);
  _masks.assign(NIMAGES, None);
  check_images();
}

void
GnomeMjTileset::check_images()
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

  // Check the size
  if (gfi->width % NACROSS != 0 || gfi->height % NDOWN != 0) {
    _image_error = ieBadSize;
    return;
  }

  _image_error = ieNone;
}

bool
GnomeMjTileset::check() const
{
  return _image_error == ieNone;
}

void
GnomeMjTileset::initialize()
{
  Gif_Image *gfi = Gif_GetImage(_gfs, 0);
  _colormap = Gif_CopyColormap(gfi->local ? gfi->local : _gfs->global);

  int border = 4, shadow = 1;
  for (Gif_Extension *gfex = _gfs->extensions; gfex; gfex = gfex->next)
    if (gfex->application && strcmp(gfex->application, "xmahjongg") == 0) {
      sscanf((char *)gfex->data, "border=%d", &border);
      sscanf((char *)gfex->data, "shadow=%d", &shadow);
    }
  _xborder = _yborder = border;
  _shadow = shadow;
  
  _width = (gfi->width / NACROSS) - _xborder;
  _height = (gfi->height / NDOWN) - _yborder;
}


static int mapping[] = {
  33, 34, 35, 36,			/* 0-3: seasons */
  38, 39, 40, 41,			/* 4-7: plants; switch w/seasons?? */
  14, 37, 13,				/* 8-10: dragons */
  9, 11, 12, 10,			/* 11-14: winds */
  0, 1, 2, 3, 4, 5, 6, 7, 8,		/* 15-23: circles */
  24, 25, 26, 27, 28, 29, 30, 31, 32,	/* 24-32: bamboo */
  15, 16, 17, 18, 19, 20, 21, 22, 23,	/* 33-41: characters */
};

void
GnomeMjTileset::draw(SwDrawable *drawable, short x, short y, short which)
{
  int w = _width + _xborder, h = _height + _yborder;
  
  if (!_images[which]) {
    Gif_Image *gfi = Gif_GetImage(_gfs, 0);
    Gif_UncompressImage(gfi);
    int pos;
    if (which == 2*NPICTURES)
      pos = 37; // white dragon
    else if (which >= NPICTURES)
      pos = mapping[which - NPICTURES] + 42;
    else
      pos = mapping[which];
    _images[which] = Gif_XSubImageColormap
      (_gifx, gfi, _colormap, (pos%NACROSS)*w, (pos/NACROSS)*h, w, h);
    _masks[which] = Gif_XSubMask
      (_gifx, gfi, (pos%NACROSS)*w, (pos/NACROSS)*h, w, h);
  }
  
  drawable->draw_image(_images[which], _masks[which], w, h, x, y);
}

void
GnomeMjTileset::draw_normal(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, which);
}

void
GnomeMjTileset::draw_lit(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, which + NPICTURES);
}

void
GnomeMjTileset::draw_obscured(const Tile *t, SwDrawable *drawable,
			      short x, short y)
{
  draw(drawable, x, y, 2*NPICTURES);
}
