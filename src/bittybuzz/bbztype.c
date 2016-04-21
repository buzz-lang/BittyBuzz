#include "bbztype.h"

/****************************************/
/****************************************/

int bbztype_cmp(const bbzobj_t* a,
                const bbzobj_t* b) {
   /* nil is smaller than anything, and equal to itself */
   if(bbztype_isnil(*a)) {
      if(bbztype_isnil(*b)) return 0;
      else return -1;
   }
   if(bbztype_isnil(*b)) return 1;
   /* If both operands are integer, perform integer comparison */
   if(bbztype_isint(*a) && bbztype_isint(*b)) {
      if(a->i.value < b->i.value) return -1;
      if(a->i.value > b->i.value) return  1;
      return 0;
   }
   /* If operands are integer or float, perform float comparison */
   if(bbztype_isint(*a) && bbztype_isfloat(*b)) {
      float x = a->i.value;
      float y = bbzfloat_tofloat(b->f.value);
      if(x < y) return -1;
      if(x > y) return  1;
      return 0;
   }
   if(bbztype_isfloat(*a) && bbztype_isint(*b)) {
      float x = bbzfloat_tofloat(a->f.value);
      float y = b->i.value;
      if(x < y) return -1;
      if(x > y) return  1;
      return 0;
   }
   if(bbztype_isfloat(*a) && bbztype_isfloat(*b)) {
      float x = bbzfloat_tofloat(a->f.value);
      float y = bbzfloat_tofloat(b->f.value);
      if(x < y) return -1;
      if(x > y) return  1;
      return 0;
   }
   /* Other cases are TODO */
   return 0;
}

/****************************************/
/****************************************/
