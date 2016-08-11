#ifndef BUTTON_HH
#define BUTTON_HH
#include "panel.hh"
#include "alarm.hh"
#include "swwidget.hh"

class Button: public AlarmHooks, public SwWidget {
  
  Gif_XContext *_gifx;
  Pixmap _normal;
  Pixmap _normal_mask;
  Pixmap _lit;
  Pixmap _lit_mask;
  
  int _state;				// 0 off, 1 lit, 2 flashing
  Alarm _flash_alarm;
  
  void change_state(int);
  bool handle_track_event(XEvent *);
  
 public:
  
  Button(SwWindow *);
  
  bool set_normal(Gif_Stream *, const char *);
  bool set_lit(Gif_Stream *, const char *);
  
  void draw();
  
  bool within(int, int) const;
  bool track(Time);

  void flash();
  void alarm();
  
};


inline void
Button::draw()
{
  if (_state > 0)
    draw_image(_lit, _lit_mask, width(), height(), 0, 0);
  else
    draw_image(_normal, _normal_mask, width(), height(), 0, 0);
}

inline bool
Button::within(int xval, int yval) const
{
  return xval >= x() && yval >= y() && xval < x() + width()
    && yval < y() + height();
}

#endif
