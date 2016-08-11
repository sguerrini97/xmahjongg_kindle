#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "swwidget.hh"


SwWidget::SwWidget(SwWindow *swwindow)
  : _swwindow(swwindow), _visible(false)
{
}

Gif_XContext *
SwWidget::get_gif_x_context()
{
  return _swwindow->get_gif_x_context();
}

void
SwWidget::draw_subimage(Pixmap src, Pixmap mask, int src_x, int src_y,
			int w, int h, int x, int y)
{
  if (x < 0) {
    w += x;
    src_x -= x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    src_y -= y;
    y = 0;
  }
  if (x + w > _width)
    w = _width - x;
  if (y + h > _height)
    h = _height - y;
  if (w >= 0 && h >= 0)
    _swwindow->draw_subimage(src, mask, src_x, src_y, w, h, x + _x, y + _y);
}

void
SwWidget::clear_area(int x, int y, int w, int h)
{
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > _width)
    w = _width - x;
  if (y + h > _height)
    h = _height - y;
  if (w >= 0 && h >= 0)
    _swwindow->clear_area(x + _x, y + _y, w, h);
}

void
SwWidget::invalidate(int x, int y, int w, int h)
{
  _swwindow->invalidate(x + _x, y + _y, w, h);
}
