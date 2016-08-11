#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "kmjts.hh"
#include "tile.hh"
#include "game.hh"
#include "swgeneral.hh"
#include <cstring>

#define NPICTURES	(Tileset::IVORY_NPICTURES)
#define NIMAGES		(2*NPICTURES + 1)

#define NACROSS		9
#define NDOWN		5

KyodaiTileset::KyodaiTileset(Gif_Stream *gfs, Gif_XContext *gifx)
  : Tileset("ivory"), _gfs(gfs),
    _gifx(gifx), _colormap(0), _hi_colormap(0), _background_colormap(0),
    _image_error(ieNone),
    _background(None), _background_mask(None)
{
  initialize_images();
  if (check()) {
    initialize();
    _gfs->refcount++;
  } else
    _gfs = 0;
}

KyodaiTileset::~KyodaiTileset()
{
  Gif_DeleteColormap(_colormap);
  Gif_DeleteColormap(_hi_colormap);
  Gif_DeleteColormap(_background_colormap);
  
  Display *display = _gifx->display;
  if (_gfs && _images.size())
    for (int i = 0; i < NIMAGES; i++) {
      if (_images[i]) XFreePixmap(display, _images[i]);
    }
  if (_background) XFreePixmap(display, _background);
  if (_background_mask) XFreePixmap(display, _background_mask);
  
  Gif_DeleteStream(_gfs);
}

void
KyodaiTileset::initialize_images()
{
  _images.assign(NIMAGES, None);
  check_images();
}

void
KyodaiTileset::check_images()
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

  _image_error = ieNone;
}

bool
KyodaiTileset::check() const
{
  return _image_error == ieNone;
}

void
KyodaiTileset::initialize()
{
  Gif_Image *gfi = Gif_GetImage(_gfs, 0);
  _colormap = Gif_CopyColormap(gfi->local ? gfi->local : _gfs->global);
  _hi_colormap = Gif_CopyColormap(_colormap);
  // highlight color: 255, 215, 0
  for (int i = 0; i < _hi_colormap->ncol; i++) {
    Gif_Color *c = &_hi_colormap->col[i];
    c->green = (c->green * 215) / 255;
    c->blue = 0;
  }

  int border = 8, shadow = 1;
  for (Gif_Extension *gfex = _gfs->extensions; gfex; gfex = gfex->next)
    if (gfex->application && strcmp(gfex->application, "xmahjongg") == 0) {
      sscanf((char *)gfex->data, "border=%u", &border);
      sscanf((char *)gfex->data, "shadow=%d", &shadow);
    }
  _xborder = _yborder = border;
  _shadow = shadow;
  
  _width = gfi->width / NACROSS;
  _height = gfi->height / NDOWN;

  /* create `_background' */
  Gif_Stream *bgfs = Gif_NewStream();
  Gif_Image *bgfi = Gif_NewImage();
  _background_colormap = Gif_NewFullColormap(10, 128);
  Gif_AddImage(bgfs, bgfi);
  bgfi->width = _width + _xborder;
  bgfi->height = _height + _yborder;
  Gif_CreateUncompressedImage(bgfi);
  bgfi->transparent = 9;
  memset(bgfi->image_data, 9, bgfi->width * bgfi->height);

  // set colors: 0-3 = vertical border, 4-7 = horizontal border
  Gif_Color *col = _background_colormap->col;
  for (int i = 0; i < 4; i++) {
    col[i].red = 211 + ((209 - 211)*i)/3; // 226,219,165
    col[i].green = 201 + ((188 - 201)*i)/3;
    col[i].blue = 148 + ((146 - 148)*i)/3;
    col[i+4].red = 181 + ((168 - 181)*i)/3;
    col[i+4].green = 170 + ((162 - 170)*i)/3;
    col[i+4].blue = 86 + ((114 - 86)*i)/3;
  }
  col[8].red = 155;
  col[8].green = 138;
  col[8].blue = 31;
  
  for (int dy = 0; dy < border; dy++) {
    int x = (_shadow & 1 ? dy : _width + _xborder - dy - 1);
    int y1 = (_shadow & 2 ? dy + 1 : border - dy);
    for (int y = 0; y < _height - 1; y++) {
      int c = (dy ? ((y/10) + (zrand()%3 - 2)) % 8 : 12);
      bgfi->img[y+y1][x] = (c >= 4 ? c - 4 : 3 - c);
    }
  }
  for (int dx = 0; dx < border; dx++) {
    int y = (_shadow & 2 ? dx : _height + _yborder - dx - 1);
    int x1 = (_shadow & 1 ? dx + 1 : border - dx);
    for (int x = 0; x < _width - 1; x++) {
      int c = (dx ? ((x/5) + (zrand()%3 - 2)) % 8 : 8);
      bgfi->img[y][x+x1] = (c >= 4 ? c : 7 - c);
    }
  }
  for (int d = 0; d < border; d++) {
    int y = (_shadow & 2 ? d : _height + _yborder - d - 1);
    int x = (_shadow & 1 ? d : _width + _xborder - d - 1);
    int y2 = (_shadow & 2 ? y+1 : y-1);
    int x2 = (_shadow & 1 ? x+1 : x-1);
    bgfi->img[y][x] = bgfi->img[y2][x] = bgfi->img[y][x2] = 8;
  }

  _background = Gif_XImageColormap(_gifx, bgfs, _background_colormap, bgfi);
  _background_mask = Gif_XMask(_gifx, bgfs, bgfi);

  Gif_DeleteStream(bgfs);
}


static int mapping[] = {
  27, 28, 29, 30,			/* 0-3: seasons */
  36, 37, 38, 39,			/* 4-7: plants; switch w/seasons?? */
  40, 41, 42,				/* 8-10: dragons */
  31, 32, 33, 34,			/* 11-14: winds */
  0, 1, 2, 3, 4, 5, 6, 7, 8,		/* 15-23: circles */
  9, 10, 11, 12, 13, 14, 15, 16, 17,	/* 24-32: bamboo */
  18, 19, 20, 21, 22, 23, 24, 25, 26,	/* 33-41: characters */
};

void
KyodaiTileset::draw(SwDrawable *drawable, short x, short y, short which)
{
  if (!_images[which]) {
    Gif_Image *gfi = Gif_GetImage(_gfs, 0);
    Gif_UncompressImage(gfi);
    Gif_Colormap *gfcm = _colormap;
    int pos;
    if (which == NIMAGES - 1)
      pos = 43; // blank
    else if (which >= NPICTURES) {
      pos = mapping[which - NPICTURES];
      gfcm = _hi_colormap;
    } else
      pos = mapping[which];
    _images[which] = Gif_XSubImageColormap
      (_gifx, gfi, gfcm,
       (pos%NACROSS)*_width, (pos/NACROSS)*_height, _width, _height);
  }
  
  drawable->draw_image(_background, _background_mask,
		       _width + _xborder, _height + _yborder, x, y);
  drawable->draw_image(_images[which], _width, _height,
		       (_shadow & 1 ? x + _xborder : x),
		       (_shadow & 2 ? y + _yborder : y));
}

void
KyodaiTileset::draw_normal(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, which);
}

void
KyodaiTileset::draw_lit(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, which + NPICTURES);
}

void
KyodaiTileset::draw_obscured(const Tile *, SwDrawable *drawable,
			     short x, short y)
{
  draw(drawable, x, y, NIMAGES - 1);
}
