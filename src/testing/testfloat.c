#include <bittybuzz/bbzfloat.h>
#include <stdio.h>

#define BASE 1.

int main() {
   for(int i = 0; i < 10; ++i) {
      printf("FLOAT %.16f -> BBZFLOAT %.16f\n",
             BASE * i, bbzfloat_tofloat(bbzfloat_fromfloat(BASE * i)));
   }
   return 0;
}
