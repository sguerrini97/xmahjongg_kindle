#ifndef SWWIDGET_HH
#define SWWIDGET_HH
#include "swgeneral.hh"

class SwWidget: public SwDrawable {

  SwWindow *_swwindow;
  
  int _x;
  int _y;
  int _width;
  int _height;
  
  bool _visible;
  
 public:
  
  SwWidget(SwWindow *);
  virtual ~SwWidget()			{ }
  
  Display *display() const		{ return _swwindow->display(); }
  Window window() const			{ return _swwindow->window(); }
  int depth() const			{ return _swwindow->depth(); }
  
  int x() const				{ return _x; }
  int y() const				{ return _y; }
  int x_pos() const			{ return _x; } // in case x is a var.
  int y_pos() const			{ return _y; } // in case y is a var.
  int width() const			{ return _width; }
  int height() const			{ return _height; }
  
  bool visible() const			{ return _visible; }
  void set_visible()			{ _visible = true; }
  
  void set_position(int x, int y)	{ _x = x; _y = y; }
  void set_size(int w, int h)		{ _width = w; _height = h; }

  Gif_XContext *get_gif_x_context();
  
  void draw_subimage(Pixmap, Pixmap, int, int, int, int, int, int);
  void clear_area(int, int, int, int);
  
  void invalidate()			{ invalidate(0, 0, _width, _height); }
  void invalidate(int x, int y, int w, int h);
  
};

#endif
