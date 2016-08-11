#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "alarm.hh"

void
AlarmHooks::alarm()
{
}

void
AlarmHooks::alarm(Alarm *)
{
  alarm();
}


Alarm *Alarm::alarms = 0;


void
Alarm::schedule(const Moment &m)
{
  if (_scheduled) unschedule();
  _moment = m;
  
  Alarm *prev = 0, *a = alarms;
  while (a && m >= a->_moment) {
    prev = a;
    a = a->_next;
  }
  
  if (prev) prev->_next = this;
  else alarms = this;
  _next = a;
  _scheduled = true;
  _dead = false;
}


void
Alarm::unschedule()
{
  if (alarms == this)
    alarms = _next;
  else {
    Alarm *prev;
    for (prev = alarms; prev->_next != this; prev = prev->_next)
      ;
    prev->_next = _next;
  }
  _scheduled = false;
}


void
Alarm::x_wait(Display *display)
{
  fd_set fds;
  int x_fd = ConnectionNumber(display);
  FD_ZERO(&fds);
  
  while (1) {
    Moment time = Moment::now();
    
    while (alarms && time >= alarms->_moment) {
      Alarm *a = alarms;
      alarms = a->_next;
      a->_scheduled = false;
      if (!a->_dead)
	a->_hook->alarm(a);
    }
    
    if (XPending(display))
      return;
    
    FD_SET(x_fd, &fds);
    int selectval;
    if (alarms) {
      struct timeval timeout = alarms->_moment - time;
      selectval = select(x_fd + 1, &fds, 0, 0, &timeout);
    } else
      selectval = select(x_fd + 1, &fds, 0, 0, 0);
    
    if (selectval > 0 && FD_ISSET(x_fd, &fds))
      return;
  }
}
