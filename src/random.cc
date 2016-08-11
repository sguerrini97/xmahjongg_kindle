#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "game.hh"
#include <cstdio>
#include <cstdlib>

#define M 1000000000
#define M_inv 1.0E-9
#define L 24
#define K 55
#define D 21

/*
  Random number generator.
    Uses the subtractive series
    
	X[n] = (X[n-55] - X[n-24]) mod M, n >= 55
	
    as suggested in Seminumerical Algorithms, exercise 3.2.2--23.
    Knuth gives a FORTRAN implementation in section 3.6 for use as a
    portable random number generator.

		Michael Coffin
*/

/*
    The constant M is machine dependent.  It must be even, and
    integer arithmetic on the range [-M,+M] must be possible.  The
    function random() returns values in the half-open interval [0,M).
    
    Minv is just 1/M.

    Other possible values for L and K can be chosen from Table 1 of
    Section 3.2.2. The constant D should be relatively prime to K and
    approximately 0.382*K.  (Don't ask me why.)

    To get a random number in the real interval [X,Y) the recommended
    formula is (X + (frandom() * (Y-X))).  For an integral interval,
    take the floor.
*/

/* current state of generator */
static uint32_t X[K];		/* Fibonacci array */
static int cur;			/* Current index in array.   */
static int curL;		/* this is always (cur - L) mod K */


/* seed the generator */
void
zrand_seed(uint32_t seed)
{
  int i;
  uint32_t j, k;
  
  /* Make zrandom() repeatable: Always reset X, cur and curL to their initial
     values. -ed */
  cur = 0;
  curL = K - L;
  for (i = 0; i < K; i++)
    X[i] = 0;
  
  /* According to Knuth, "This computes a Fibonacci-like sequence; */
  /* multiplication of indices by 21 [the constant D] spreads the */
  /* values around so as to alleviate initial nonrandomness */
  /* problems..." */ 
  seed = seed % M;
  
  X[0] = j = seed;
  k = 1;
  
  for (i = 1; i < K; i++) {
    int which = (D*i) % K;
    X[which] = k;
    if (j < k) k -= M;
    k = j - k;
    j = X[which];
  }
  
  /* `warm up' the generator. Three was good enough for Knuth... */
  for (i = 0; i < 5 * K; i++)
    zrand();
}


/* return next value in the sequence */
uint32_t
zrand()
{
  uint32_t result;
  
  result = X[cur] < X[curL] ? M : 0;
  result += X[cur] - X[curL];
  X[cur] = result;
  
  cur++; if (cur == K) cur = 0;
  curL++; if (curL == K) curL = 0;
  
  return result;
}
