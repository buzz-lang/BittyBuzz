#include "bbzdarray.h"

/****************************************/
/****************************************/

int bbzdarray_new(bbzheap_idx_t* d) {
   /* Allocation of a new array */
   if (!bbzheap_obj_alloc(&vm->heap, BBZTYPE_TABLE, d)) return 0;
   /* Set the bit that tells it's a dynamic array */
   bbzheap_obj_at(&vm->heap, *d)->t.mdata |= 0x04;
   return 1;
}

/****************************************/
/****************************************/

void bbzdarray_destroy(bbzheap_idx_t d) {
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
   while (1) {
      if (bbzdarray_iscloned(da)) {
         for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (bbzheap_aseg_elem_isvalid(sd->values[i])) {
               obj_makeinvalid(*bbzheap_obj_at(&vm->heap, bbzheap_aseg_elem_get(sd->values[i])));
            }
         }
      }
      tseg_makeinvalid(*sd);
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(&vm->heap, si);
   }
   obj_makeinvalid(*((bbzobj_t*)da));
}

/****************************************/
/****************************************/

int bbzdarray_get(bbzheap_idx_t d,
                  uint16_t idx,
                  bbzheap_idx_t* v) {
   if (idx >= bbzdarray_size(d)) return 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
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
      sd = bbzheap_aseg_at(&vm->heap, si);
      ++i;
   }
   return 0;
}

/****************************************/
/****************************************/

int bbzdarray_set(bbzheap_idx_t d,
                  uint16_t idx,
                  bbzheap_idx_t v) {
   if (idx >= bbzdarray_size(d)) return 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
   while (1) {
      if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
         if (bbzheap_aseg_elem_isvalid(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])) {
            uint16_t o = v;
            if (bbzdarray_iscloned(da)) {
               obj_makeinvalid(*bbzheap_obj_at(&vm->heap, bbzheap_aseg_elem_get(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])));
               if (bbzheap_obj_alloc(&vm->heap, BBZTYPE_NIL, &o)) {
                  bbzheap_obj_copy(&vm->heap, v, o);
               }
            }
            bbzheap_aseg_elem_set(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)], o);
            return 1;
         }
         break;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(&vm->heap, si);
      ++i;
   }
   return 0;
}

/****************************************/
/****************************************/

int bbzdarray_pop(bbzheap_idx_t d) {
   uint16_t idx = bbzdarray_size(d);
   if (idx == 0) return 0;
   --idx;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
   bbzheap_aseg_t* prevsd = NULL;
   while (1) {
      if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
         if (bbzheap_aseg_elem_isvalid(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])) {
            if (bbzdarray_iscloned(da)) {
               obj_makeinvalid(*bbzheap_obj_at(&vm->heap, bbzheap_aseg_elem_get(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)])));
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
      sd = bbzheap_aseg_at(&vm->heap, si);
      ++i;
   }
   return 1;
}

/****************************************/
/****************************************/

int bbzdarray_push(bbzheap_idx_t d,
                   bbzheap_idx_t v) {
   uint16_t idx = bbzdarray_size(d);
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t i = 0;
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
   while (1) {
      if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
         uint16_t o = v;
         if (bbzdarray_iscloned(da)) {
            if(!bbzheap_obj_alloc(&vm->heap, BBZTYPE_NIL, &o)) return 0;
            bbzheap_obj_copy(&vm->heap, v, o);
         }
         bbzheap_aseg_elem_set(sd->values[idx%(2*BBZHEAP_ELEMS_PER_TSEG)], o);
         return 1;
      }
      if (!bbzheap_aseg_hasnext(sd)) {
         uint16_t o;
         if (!bbzheap_aseg_alloc(&vm->heap, &o)) return 0;
         bbzheap_aseg_next_set(sd, o);
      }
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(&vm->heap, si);
      ++i;
   }
   return 1;
}

/****************************************/
/****************************************/

uint16_t bbzdarray_size(bbzheap_idx_t d) {
   uint16_t size = 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
   while (1) {
      for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
         if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
            return size;
         }
         ++size;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(&vm->heap, si);
   }
   return size;
}

/****************************************/
/****************************************/

int bbzdarray_clone(bbzheap_idx_t d,
                    bbzheap_idx_t* newd) {
   bbzdarray_new(newd);
   /* Set the bit that tells it's cloned */
   bbzdarray_mark_cloned((bbzdarray_t*)bbzheap_obj_at(&vm->heap, *newd));
   uint16_t idx = bbzdarray_size(d);
   uint16_t v;
   for (int i = 0; i < idx; ++i) {
      bbzdarray_get(d, i, &v);
      if (!bbzdarray_push(*newd, v)) return 0;
   }
   return 1;
}

/****************************************/
/****************************************/

void bbzdarray_clear(bbzheap_idx_t d) {
   //TODO Optimize this function
   for (int i = bbzdarray_size(d); i > 0; --i) {
      bbzdarray_pop(d);
   }
   bbzdarray_unmark_cloned((bbzdarray_t*)bbzheap_obj_at(&vm->heap, d));
}

/****************************************/
/****************************************/

void bbzdarray_foreach(bbzheap_idx_t d,
                       bbzdarray_elem_funp fun,
                       void* params) {
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
   while (1) {
      for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
         if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
            return;
         }
         fun(d, bbzheap_aseg_elem_get(sd->values[i]), params);
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(&vm->heap, si);
   }
}

/****************************************/
/****************************************/

uint16_t bbzdarray_find(bbzheap_idx_t d,
                        bbzdarray_elem_cmpp cmp,
                        bbzheap_idx_t data) {
   uint16_t pos = 0;
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(&vm->heap, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(&vm->heap, si);
   while (1) {
      for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
         if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
            break;
         }
         if (cmp(bbzheap_obj_at(&vm->heap, bbzheap_aseg_elem_get(sd->values[i])), bbzheap_obj_at(&vm->heap, data)) == 0) {
            return pos;
         }
         ++pos;
      }
      if (!bbzheap_aseg_hasnext(sd)) break;
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(&vm->heap, si);
   }
   return pos;
}

/****************************************/
/****************************************/

int bbzdarray_lambda_alloc(bbzheap_idx_t d, uint8_t* l) {
   /* Look for empty slot */
   for(uint8_t i = 0;
	   i < RESERVED_ACTREC_MAX;
	   ++i)
	  if(!bbzheap_obj_isvalid(*bbzheap_obj_at(&vm->heap, i))) {
		 /* Empty slot found */
		 bbzobj_t* x = bbzheap_obj_at(&vm->heap, i);
		 /* Set valid bit and type */
		 obj_makevalid(*x);
		 bbztype_cast(*x, BBZTYPE_TABLE);
		 /* Set the bit that tells it's a dynamic array */
		 x->t.mdata |= 0x04;
		 /* Set result */
		 *l = i;
		 /* Allocate an array segment */
		 if(!bbzheap_aseg_alloc(&vm->heap, &(x->t.value))) return 0;

		 bbzdarray_mark_cloned(&x->t);
		 uint16_t idx = bbzdarray_size(d);
		 uint16_t v;
		 for (int j = 0; j < idx; ++j) {
		    bbzdarray_get(d, j, &v);
		    if (!bbzdarray_push(*l, v)) return 0;
		 }
		 /* Success */
		 return 1;
	  }
   /* No empty slot found, we're out of reserved memory! */
   return 0;
}