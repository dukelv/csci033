/* Testing Code */

#include <limits.h>
#include <math.h>

/* Routines used by floation point test code */

/* Convert from bit level representation to floating point number */
float u2f(unsigned u) {
  union {
    unsigned u;
    float f;
  } a;
  a.u = u;
  return a.f;
}

/* Convert from floating point number to bit-level representation */
unsigned f2u(float f) {
  union {
    unsigned u;
    float f;
  } a;
  a.f = f;
  return a.u;
}

int test_bitAnd(int x, int y) {
 return x&y;
}
int test_conditional(int x, int y, int z) {
 return x?y:z;
}
int test_negate(int x) {
 return -x;
}
int test_isEqual(int x, int y) {
 return x == y;
}
int test_divpwr2(int x, int n) {
 int p2n = 1<<n;
 return x/p2n;
}
int test_addOK(int x, int y)
{
 long long lsum = (long long) x + y;
 return lsum == (int) lsum;
}







int test_absVal(int x) {
 return (x < 0) ? -x : x;
}
int test_float_f2i(unsigned uf) {
 float f = u2f(uf);
 int x = (int) f;
 return x;
}
