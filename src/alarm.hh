#ifndef ALARM_HH
#define ALARM_HH
#include "moment.hh"
#include <X11/Xlib.h>
class Alarm;

class AlarmHooks {
  
 public:

  virtual ~AlarmHooks()			{ }
  
  virtual void alarm();
  virtual void alarm(Alarm *);
  
};

class Alarm {
  
  bool _dead;
  bool _scheduled;
  Moment _moment;
  AlarmHooks *_hook;
  Alarm *_next;
  
  static Alarm *alarms;
  void unschedule();
  
 public:
  
  Alarm()				: _scheduled(0), _hook(0) { }
  Alarm(AlarmHooks *h)			: _scheduled(0), _hook(h) { }
  ~Alarm()				{ if (_scheduled) unschedule(); }
  
  const Moment &moment() const		{ return _moment; }
  bool dead() const			{ return _dead || !_scheduled; }
  
  void schedule(const Moment &);
  void kill()				{ _dead = true; }
  
  static void x_wait(Display *);
  
};

#endif
