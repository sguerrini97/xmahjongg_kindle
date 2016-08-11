#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "swgeneral.hh"
#include <cstdio>


SwDrawable::~SwDrawable()
{
}

void
SwDrawable::clear_area(int, int, int, int)
{
}

void
SwDrawable::invalidate(int, int, int, int)
{
}

//

SwWindow::SwWindow(Display *display, Window window, int depth)
  : _display(display), _window(window), _depth(depth), _gifx(0)
{
  if (_depth <= 0) {
    XWindowAttributes attr;
    XGetWindowAttributes(_display, _window, &attr);
    _depth = attr.depth;
  }
  _copy_gc = XCreateGC(_display, _window, 0, NULL);
  _masked_image_gc = XCreateGC(_display, _window, 0, NULL);
}

SwWindow::~SwWindow()
{
  XFreeGC(_display, _copy_gc);
  XFreeGC(_display, _masked_image_gc);
  delete _gifx;
}

Gif_XContext *
SwWindow::get_gif_x_context()
{
  if (!_gifx) _gifx = Gif_NewXContext(_display, _window);
  return _gifx;
}

void
SwWindow::draw_subimage(Pixmap source, Pixmap mask,
			int src_x, int src_y, int width, int height,
			int x, int y)
{
  if (source && width > 0 && height > 0) {
    GC image_gc;
    if (mask) {
      image_gc = _masked_image_gc;
      XSetClipMask(_display, image_gc, mask);
      XSetClipOrigin(_display, image_gc, x - src_x, y - src_y);
    } else
      image_gc = _copy_gc;
    XCopyArea(_display, source, _window, image_gc, src_x, src_y,
	      width, height, x, y);
  }
}

void
SwWindow::clear_area(int x, int y, int width, int height)
{
  if (width > 0 && height > 0)
    XClearArea(_display, _window, x, y, width, height, False);
}

//

SwClippedWindow::SwClippedWindow(Display *display, Window window, int depth)
  : SwWindow(display, window, depth), _clipping(false)
{
}

void
SwClippedWindow::intersect_clip(int left, int top, int w, int h)
{
  int right = left + w;
  int bottom = top + h;
  if (_clipping) {
    if (left < _clip_left) left = _clip_left;
    if (right > _clip_right) right = _clip_right;
    if (top < _clip_top) top = _clip_top;
    if (bottom > _clip_bottom) bottom = _clip_bottom;
  }
  _clip_left = left;
  _clip_right = right;
  _clip_top = top;
  _clip_bottom = bottom;
  _clipping = true;
}

void
SwClippedWindow::union_clip(int left, int top, int w, int h)
{
  int right = left + w;
  int bottom = top + h;
  if (_clipping) {
    if (left > _clip_left) left = _clip_left;
    if (right < _clip_right) right = _clip_right;
    if (top > _clip_top) top = _clip_top;
    if (bottom < _clip_bottom) bottom = _clip_bottom;
  }
  _clip_left = left;
  _clip_right = right;
  _clip_top = top;
  _clip_bottom = bottom;
  _clipping = true;
}

void
SwClippedWindow::do_clip(int &x, int &y, int &width, int &height) const
{
  if (_clipping) {
    if (x + width > _clip_right)
      width = _clip_right - x;
    if (y + height > _clip_bottom)
      height = _clip_bottom - y;
    if (x < _clip_left) {
      width -= _clip_left - x;
      x = _clip_left;
    }
    if (y < _clip_top) {
      height -= _clip_top - y;
      y = _clip_top;
    }
  }
}

void
SwClippedWindow::draw_subimage(Pixmap source, Pixmap mask,
		       int src_x, int src_y, int width, int height,
		       int x, int y)
{
  short old_x = x, old_y = y;
  do_clip(x, y, width, height);
  if (width > 0 && height > 0)
    SwWindow::draw_subimage(source, mask, src_x + x - old_x, src_y + y - old_y,
			    width, height, x, y);
}

void
SwClippedWindow::clear_area(int x, int y, int width, int height)
{
  do_clip(x, y, width, height);
  SwWindow::clear_area(x, y, width, height);
}

void
SwClippedWindow::invalidate(int x, int y, int width, int height)
{
  do_clip(x, y, width, height);
  SwWindow::invalidate(x, y, width, height);
}

//

SwImage::~SwImage()
{
  if (_source) XFreePixmap(_display, _source);
  if (_mask) XFreePixmap(_display, _mask);
}

void
SwImage::draw(SwDrawable *drawable, int x, int y)
{
  if (_source)
    drawable->draw_subimage(_source, _mask, 0, 0, _width, _height, x, y);
}


SwGifImage::~SwGifImage()
{
  if (_gfi) {
    _gfi->refcount--;
    if (!_gfi->refcount) Gif_DeleteImage(_gfi);
  }
  _gfs->refcount--;
  if (!_gfs->refcount) Gif_DeleteStream(_gfs);
}

void
SwGifImage::initialize(Gif_Stream *gfs, Gif_Image *gfi)
{
  _gfs = gfs;
  _gfi = gfi;
  _gfs->refcount++;
  if (_gfi) _gfi->refcount++;
}

void
SwGifImage::create_pixmaps(SwDrawable *drawable)
{
  if (!_gfi) return;
  Gif_XContext *gifx = drawable->get_gif_x_context();
  Pixmap source = Gif_XImage(gifx, _gfs, _gfi);
  Pixmap mask = None;
  if (_gfi->transparent >= 0)
    mask = Gif_XMask(gifx, _gfs, _gfi);
  set_image(gifx->display, source, mask, _gfi->width, _gfi->height);
  _made = true;
}

void
SwGifImage::draw(SwDrawable *drawable, int x, int y)
{
  if (!_made)
    create_pixmaps(drawable);
  if (_source)
    drawable->draw_subimage(_source, _mask, 0, 0, _width, _height, x, y);
}
