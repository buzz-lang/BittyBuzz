#include "bbzdarray.h"

#define bbzdarray_mark_cloned(d) (d)->mdata |= 0x02
#define bbzdarray_unmark_cloned(d) (d)->mdata &= ~0x02
#define bbzdarray_iscloned(d) ((d)->mdata & 0x02)

/* from bbzheap.c */
#define obj_makevalid(x)   (x).o.mdata |= 0x10
#define obj_makeinvalid(x) (x).o.mdata &= 0xEF
#define tseg_makevalid(s)   (s).mdata = 0xFFFF // Make the segment valid AND set next to -1
#define tseg_makeinvalid(s) (s).mdata &= 0x7FFF

/****************************************/
/****************************************/

int bbzdarray_new(bbzheap_t* h,
                  bbzheap_idx_t* d) {
   /* Allocation of a new array */
   bbzheap_obj_alloc(h, BBZTYPE_TABLE, d);
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, *d);
   /* Set the bit that tells it's a dynamic array */
   da->mdata |= 0x04;
   /* Allocate a new array segment */
   bbzheap_aseg_alloc(h, &(da->value));
}

/****************************************/
/****************************************/

void bbzdarray_destroy(bbzheap_t* h,
                       bbzheap_idx_t d) {
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   while (1) {
      if (bbzdarray_iscloned(da)) {
         for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (bbzheap_aseg_elem_isvalid(sd->values[i])) {
               obj_makeinvalid(*bbzheap_obj_at(h, bbzheap_aseg_elem_get(sd->values[i])));
            }
         }
      }
      tseg_makeinvalid(*sd);
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
   }
   obj_makeinvalid(*((bbzobj_t*)da));
}

/****************************************/
/****************************************/

int bbzdarray_get(bbzheap_t* h,
                  bbzheap_idx_t d,
                  uint16_t idx,
                  bbzheap_idx_t* v) {
   if (idx >= bbzdarray_size(h, d)) return 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   while (1) {
      if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
         if (bbzheap_aseg_elem_isvalid(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])) {
            *v = bbzheap_aseg_elem_get(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)]);
            return 1;
         }
         break;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
      ++i;
   }
   return 0;
}

/****************************************/
/****************************************/

int bbzdarray_set(bbzheap_t* h,
                  bbzheap_idx_t d,
                  uint16_t idx,
                  bbzheap_idx_t v) {
   if (idx >= bbzdarray_size(h, d)) return 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   while (1) {
      if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
         if (bbzheap_aseg_elem_isvalid(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])) {
            uint16_t o = v;
            if (bbzdarray_iscloned(da)) {
               obj_makeinvalid(*bbzheap_obj_at(h, bbzheap_aseg_elem_get(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])));
               if (bbzheap_obj_alloc(h, BBZTYPE_NIL, &o)) {
                  bbzheap_obj_copy(h, v, o);
               }
            }
            bbzheap_aseg_elem_set(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)], o);
            return 1;
         }
         break;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
      ++i;
   }
   return 0;
}

/****************************************/
/****************************************/

int bbzdarray_pop(bbzheap_t* h,
                  bbzheap_idx_t d) {
   uint16_t idx = bbzdarray_size(h, d);
   if (idx == 0) return 0;
   --idx;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   bbzheap_aseg_t* prevsd = NULL;
   while (1) {
      if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
         if (bbzheap_aseg_elem_isvalid(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])) {
            if (bbzdarray_iscloned(da)) {
               obj_makeinvalid(*bbzheap_obj_at(h, bbzheap_aseg_elem_get(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])));
            }
            sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)] &= ~MASK_VALID_SEG_ELEM;
         }
         if (idx%(2*BBZHEAP_ELEMS_PER_TSEG) == 0) {
            if (prevsd != NULL) {
               tseg_makeinvalid(*sd);
               bbzheap_aseg_next_set(prevsd, NO_NEXT);
            }
         }
         break;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      prevsd = sd;
      sd = bbzheap_aseg_at(h, si);
      ++i;
   }
   return 1;
}

/****************************************/
/****************************************/

int bbzdarray_push(bbzheap_t* h,
                   bbzheap_idx_t d,
                   bbzheap_idx_t v) {
   uint16_t idx = bbzdarray_size(h, d);
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   while (1) {
      if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
         uint16_t o = v;
         if (bbzdarray_iscloned(da)) {
            if (bbzheap_obj_alloc(h, BBZTYPE_NIL, &o)) {
               bbzheap_obj_copy(h, v, o);
            }
            else return 0;
         }
         bbzheap_aseg_elem_set(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)], o);
         return 1;
      }
      if (!bbzheap_aseg_hasnext(sd)) {
         uint16_t o;
         if (!bbzheap_aseg_alloc(h, &o)) return 0;
         bbzheap_aseg_next_set(sd, o);
      }
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
      ++i;
   }
   return 1;
}

/****************************************/
/****************************************/

uint16_t bbzdarray_size(bbzheap_t* h,
                        bbzheap_idx_t d) {
   uint16_t size = 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   while (1) {
      for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
         if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
            return size;
         }
         ++size;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
   }
   return size;
}

/****************************************/
/****************************************/

int bbzdarray_clone(bbzheap_t* h,
                    bbzheap_idx_t d,
                    bbzheap_idx_t* newd) {
   bbzdarray_new(h, newd);
   /* Set the bit that tells it's cloned */
   bbzdarray_mark_cloned((bbzdarray_t*)bbzheap_obj_at(h, *newd));
   uint16_t idx = bbzdarray_size(h, d);
   uint16_t v;
   for (int i = 0; i < idx; ++i) {
      bbzdarray_get(h, d, i, &v);
      if (!bbzdarray_push(h, *newd, v)) {
         return 0;
      }
   }
   return 1;
}

/****************************************/
/****************************************/

void bbzdarray_clear(bbzheap_t* h,
                     bbzheap_idx_t d) {
   //TODO Optimize this function
   for (int i = bbzdarray_size(h,d); i > 0; --i) {
      bbzdarray_pop(h, d);
   }
   bbzdarray_unmark_cloned((bbzdarray_t*)bbzheap_obj_at(h, d));
}

/****************************************/
/****************************************/

void bbzdarray_foreach(bbzheap_t* h,
                       bbzheap_idx_t d,
                       bbzdarray_elem_funp fun,
                       void* params) {
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   while (1) {
      for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
         if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
            return;
         }
         fun(h, d, bbzheap_aseg_elem_get(sd->values[i]), params);
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
   }
}

/****************************************/
/****************************************/

uint16_t bbzdarray_find(bbzheap_t* h,
                        bbzheap_idx_t d,
                        bbzdarray_elem_cmpp cmp,
                        bbzheap_idx_t data) {
   uint16_t pos = 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   while (1) {
      for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
         if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
            break;
         }
         if (cmp(bbzheap_obj_at(h, bbzheap_aseg_elem_get(sd->values[i])), bbzheap_obj_at(h, data)) == 0) {
            return pos;
         }
         ++pos;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
   }
   return pos;
}