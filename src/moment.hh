#ifndef MOMENT_HH
#define MOMENT_HH
#include <X11/Xos.h>

class Moment {
  
  long _sec;
  long _usec;
  
  static const long MicroPerSec = 1000000;
  static Moment genesis;
  
  Moment(Moment *);
  
 public:
  
  Moment()				{ }
  Moment(long sec, long usec = 0)	: _sec(sec), _usec(usec) { }
  Moment(const struct timeval &t)	: _sec(t.tv_sec), _usec(t.tv_usec) { }
  
  static Moment now();
  
  operator struct timeval() const;
  long sec() const			{ return _sec; }
  long usec() const			{ return _usec; }
  
  Moment &operator+=(const Moment &);
  Moment &operator-=(const Moment &);
  friend Moment operator+(const Moment &, const Moment &);
  friend Moment operator-(const Moment &, const Moment &);
  
  friend bool operator>=(const Moment &, const Moment &);
  friend bool operator>(const Moment &, const Moment &);
  
};


inline Moment &
Moment::operator+=(const Moment &a)
{
  _sec += a._sec;
  if ((_usec += a._usec) >= MicroPerSec) {
    _sec++;
    _usec -= MicroPerSec;
  }
  return *this;
}

inline Moment &
Moment::operator-=(const Moment &a)
{
  _sec -= a._sec;
  if ((_usec -= a._usec) < 0) {
    _sec--;
    _usec += MicroPerSec;
  }
  return *this;
}

inline Moment
operator+(const Moment &a, const Moment &b)
{
  Moment c = a;
  return c += b;
}

inline Moment
operator-(const Moment &a, const Moment &b)
{
  Moment c = a;
  return c -= b;
}

inline bool
operator>=(const Moment &a, const Moment &b)
{
  return a._sec > b._sec || (a._sec == b._sec && a._usec >= b._usec);
}

inline bool
operator>(const Moment &a, const Moment &b)
{
  return a._sec > b._sec || (a._sec == b._sec && a._usec > b._usec);
}

inline
Moment::operator struct timeval() const
{
  struct timeval t;
  t.tv_sec = _sec;
  t.tv_usec = _usec;
  return t;
}

#endif
