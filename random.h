#include <math.h>
#include <sys/time.h>

/* Certaim details inspired by McStas/McXtrace - see mcstas.org */

/* Random number algorithm from: http://www.helsbreth.org/random/rng_kiss.html */
void kiss_srandom(unsigned long state[7], unsigned long seed) {
  if (seed == 0) seed = 1;
  state[0] = seed | 1; // x
  state[1] = seed | 2; // y
  state[2] = seed | 4; // z
  state[3] = seed | 8; // w
  state[4] = 0;        // carry
}
unsigned long kiss_random(unsigned long state[7]) {
    state[0] = state[0] * 69069 + 1;
    state[1] ^= state[1] << 13;
    state[1] ^= state[1] >> 17;
    state[1] ^= state[1] << 5;
    state[5] = (state[2] >> 2) + (state[3] >> 3) + (state[4] >> 2);
    state[6] = state[3] + state[3] + state[2] + state[4];
    state[2] = state[3];
    state[3] = state[6];
    state[4] = state[5] >> 30;
    return state[0] + state[1] + state[3];
}


#ifndef ULONG_MAX
#  define ULONG_MAX ((unsigned long)0xffffffffffffffffUL)
#endif
typedef unsigned long randstate_t;
randstate_t _hash(randstate_t x) {
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x);
  return x;
}


// single-threaded easy-to-use random numbers
randstate_t g_state[7];
#define random() kiss_random(g_state)

// easy rand init
void rand_init() {
  struct timeval tm;
  gettimeofday(&tm, NULL);
  unsigned long seed = _hash((unsigned long) tm.tv_sec*1000000 + tm.tv_usec);

  kiss_srandom(g_state, seed);
}


// return a random number between 0 and 1
double rand01() {
  double randnum;
  randnum = (double) random();
  // TODO: can we mult instead of div?
  randnum /= (double) ULONG_MAX + 1;
  return randnum;
}
// return a random number between 1 and -1
double randpm1() {
  double randnum;
  randnum = (double) random();
  randnum /= ((double) ULONG_MAX + 1) / 2;
  randnum -= 1;
  return randnum;
}
// return a random number between 0 and max
double rand0max(double max) {
  double randnum;
  randnum = (double) random();
  randnum /= ((double) ULONG_MAX + 1) / max;
  return randnum;
}
int randminmax_i(int min, int max) {
  assert(max > min);
  return random() % (max - min + 1) + min;
}

// return a random number between min and max
double randminmax(double min, double max) {
  return rand0max(max - min) + max;
}
// return a random number from -1 to 1 with triangle distribution
double randtriangle() {
  double randnum = rand01();
  if (randnum>0.5)
    return(1-sqrt(2*(randnum-0.5)));
  else
    return(sqrt(2*randnum)-1);
}
// from normal law
double randnorm()
{
  static double v1, v2, s;
  static int phase = 0;
  double X, u1, u2;

  if(phase == 0)
  {
    do
    {
      u1 = rand01();
      u2 = rand01();
      v1 = 2*u1 - 1;
      v2 = 2*u2 - 1;
      s = v1*v1 + v2*v2;
    } while(s >= 1 || s == 0);

    X = v1*sqrt(-2*log(s)/s);
  }
  else
  {
    X = v2*sqrt(-2*log(s)/s);
  }

  phase = 1 - phase;
  return X;
}
// another one
double gaussian_double() {
  double x, y, r;

  do {
    x = 2.0 * rand01() - 1.0;
    y = 2.0 * rand01() - 1.0;
    r = x*x + y*y;
  } while (r == 0.0 || r >= 1.0);

  return x * sqrt((-2.0 * log(r)) / r);
}
