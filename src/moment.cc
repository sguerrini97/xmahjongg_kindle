#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "moment.hh"
#include <cassert>
#include <sys/time.h>

#ifdef X_GETTIMEOFDAY
#define ET_GETTIMEOFDAY(t) X_GETTIMEOFDAY(t)
#elif GETTIMEOFDAY_PROTO == 0
extern "C" int gettimeofday(struct timeval *, struct timezone *);
#define ET_GETTIMEOFDAY(t) gettimeofday((t), 0)
#elif GETTIMEOFDAY_PROTO == 1
#define ET_GETTIMEOFDAY(t) gettimeofday((t))
#else
#define ET_GETTIMEOFDAY(t) gettimeofday((t), 0)
#endif


Moment Moment::genesis((Moment *)0);


Moment::Moment(Moment *m)
{
  assert(m == 0);
  struct timeval tv;
  ET_GETTIMEOFDAY(&tv);
  _sec = tv.tv_sec;
  _usec = tv.tv_usec;
}


Moment
Moment::now()
{
  Moment nowish((Moment *)0);
  return nowish -= genesis;
}
