#include "bbzheap.h"
#include "bbztype.h"
#include <stdio.h>

/****************************************/
/****************************************/

#define gc_hasmark(x) ((x).o.mdata & 0x08)
#define gc_mark(x)    (x).o.mdata |= 0x08
#define gc_unmark(x)  (x).o.mdata &= 0xF7

/****************************************/
/****************************************/

void bbzheap_clear() {
   vm.heap.rtobj = vm.heap.data + RESERVED_ACTREC_MAX * sizeof(bbzobj_t);
   vm.heap.ltseg = vm.heap.data + BBZHEAP_SIZE;
   for(int16_t i = BBZHEAP_SIZE-1; i >= 0; --i) {
       vm.heap.data[i] = 0;
   }
}

/****************************************/
/****************************************/

uint8_t bbzheap_obj_alloc(uint8_t t,
                          bbzheap_idx_t* o) {
   /* Look for empty slot */
   for(int16_t i = (vm.heap.rtobj - vm.heap.data) / sizeof(bbzobj_t) - 1;
       i >= RESERVED_ACTREC_MAX;
       --i)
      if(!bbzheap_obj_isvalid(*bbzheap_obj_at(i))) {
         /* Empty slot found */
         bbzobj_t* x = bbzheap_obj_at(i);
         /* Set valid bit and type */
         obj_makevalid(*x);
         bbztype_cast(*x, t);
         /* Set result */
         *o = i;
         /* Take care of special initialisations */
         if (t == BBZTYPE_TABLE) {
            bbztable_t* x = &bbzheap_obj_at(*o)->t;
            if(!bbzheap_tseg_alloc(&x->value)) return 0;
         }
         else if (bbztype_isclosure(*x)) {
            bbzclosure_unmake_lambda(*x);
            ((bbzlclosure_t*)x)->value.actrec = 0xFF; // Default activation record
         }
         /* Success */
         return 1;
      }
   /* No empty slot found, must create a new one */
   /* ...but first, make sure there is room */
   if(vm.heap.rtobj + sizeof(bbzobj_t) > vm.heap.ltseg) return 0;
   /* Set result */
   *o = (vm.heap.rtobj - vm.heap.data) / sizeof(bbzobj_t);
   /* Set valid bit and type */
   obj_makevalid(*((bbzobj_t*)vm.heap.rtobj));
   bbztype_cast(*((bbzobj_t*)vm.heap.rtobj), t);
   /* Make room */
   vm.heap.rtobj += sizeof(bbzobj_t);
   /* Take care of special initialisations */
   if (t == BBZTYPE_TABLE) {
      bbztable_t* x = &bbzheap_obj_at(*o)->t;
      if(!bbzheap_tseg_alloc(&(x->value))) return 0;
   }
   else if (bbztype_isclosure(*bbzheap_obj_at(*o))) {
      bbzlclosure_t* x = (bbzlclosure_t*)bbzheap_obj_at(*o);
      x->value.actrec = 0xFF; // Default activation record
   }
   return 1;
}

/****************************************/
/****************************************/

uint8_t bbzheap_tseg_alloc(bbzheap_idx_t* s) {
   /* Look for empty slot */
   for(int16_t i = (vm.heap.data + BBZHEAP_SIZE - vm.heap.ltseg) / sizeof(bbzheap_tseg_t) - 1;
       i >= 0;
       --i)
      if(!bbzheap_tseg_isvalid(*bbzheap_tseg_at(i))) {
         /* Empty slot found */
         bbzheap_tseg_t* x = bbzheap_tseg_at(i);
         /* Set valid bit of segment and -1 index for next */
         tseg_makevalid(*x);
         /* Invalidate keys and values */
         for(uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            x->keys[i] = 0;
            x->values[i] = 0;
         }
         /* Set result */
         *s = i;
         /* Success */
         return 1;
      }
   /* Make sure there is room */
   if(vm.heap.ltseg - sizeof(bbzheap_tseg_t) < vm.heap.rtobj) return 0;
   /* Set result */
   *s = (vm.heap.data + BBZHEAP_SIZE - vm.heap.ltseg) / sizeof(bbzheap_tseg_t);
   /* Update pointer to leftmost invalid segment */
   vm.heap.ltseg -= sizeof(bbzheap_tseg_t);
   /* Set valid bit of segment and zero index for next */
   tseg_makevalid(*(bbzheap_tseg_t*)(vm.heap.ltseg));
   /* Invalidate keys and values */
   for(uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
      ((bbzheap_tseg_t*)vm.heap.ltseg)->keys[i] = 0;
      ((bbzheap_tseg_t*)vm.heap.ltseg)->values[i] = 0;
   }
   return 1;
}

/****************************************/
/****************************************/

void bbzheap_gc_mark(bbzheap_idx_t obj) {
   if (gc_hasmark(*bbzheap_obj_at(obj))) return;
   /* Mark gc bit */
   gc_mark(*bbzheap_obj_at(obj));
   /* If it's a table, go through it and mark all associated objects */
   if(bbztype_istable(*bbzheap_obj_at(obj))) {
      /* Segment index in heap */
      bbzheap_idx_t si = bbzheap_obj_at(obj)->t.value;
      if(bbztype_isdarray(*bbzheap_obj_at(obj))) {
         /* Actual segment data in heap */
         bbzheap_aseg_t* sd = bbzheap_aseg_at(si);
         /* Go through the segments */
         while(1) {
            for(uint8_t j = 0; j < 2*BBZHEAP_ELEMS_PER_TSEG; ++j)
               if(bbzheap_aseg_elem_isvalid(sd->values[j])) {
                  bbzheap_gc_mark(bbzheap_aseg_elem_get(sd->values[j]));
               }
            if(!bbzheap_aseg_hasnext(sd)) break;
            si = bbzheap_aseg_next_get(sd);
            sd = bbzheap_aseg_at(si);
         }
      }
      else {
         /* Actual segment data in heap */
         bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
         /* Go through the segments */
         while(1) {
            for(uint8_t j = 0; j < BBZHEAP_ELEMS_PER_TSEG; ++j)
               if(bbzheap_tseg_elem_isvalid(sd->keys[j])) {
                  bbzheap_gc_mark(bbzheap_tseg_elem_get(sd->keys[j]));
                  bbzheap_gc_mark(bbzheap_tseg_elem_get(sd->values[j]));
               }
            if(!bbzheap_tseg_hasnext(sd)) break;
            si = bbzheap_tseg_next_get(sd);
            sd = bbzheap_tseg_at(si);
         }
      }
   }
   else if (bbztype_isclosurelambda(*bbzheap_obj_at(obj))) {
      bbzheap_gc_mark(bbzheap_obj_at(obj)->l.value.actrec);
   }
}

void bbzheap_gc(bbzheap_idx_t* st,
                uint16_t sz) {
   /* Set all gc bits to zero */
   for(int16_t i = (vm.heap.rtobj - vm.heap.data) / sizeof(bbzobj_t) - 1; i >= 0; --i)
      gc_unmark(*bbzheap_obj_at(i));
   /* Go through the stack and set the gc bit of valid variables */
   for(uint16_t i = 0; i < sz; ++i) {
      /* Mark gc bit */
      bbzheap_gc_mark(st[i]);
   }
   /* Go through the objects; invalidate those with 0 gc bit */
   for(int16_t i = (vm.heap.rtobj - vm.heap.data) / sizeof(bbzobj_t) - 1; i >= 0; --i) {
      if(!gc_hasmark(*bbzheap_obj_at(i))) {
         /* Invalidate object */
         obj_makeinvalid(*bbzheap_obj_at(i));
         /* If it's a table, invalidate its segments too */
         if(bbztype_istable(*bbzheap_obj_at(i))) {
            /* Segment index in heap */
            bbzheap_idx_t si = bbzheap_obj_at(i)->t.value;
            /* Actual segment data in heap */
            bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
            /* Go through the segments and invalidate them all */
            while(1) {
               tseg_makeinvalid(*sd);
               if(!bbzheap_tseg_hasnext(sd)) break;
               si = bbzheap_tseg_next_get(sd);
               sd = bbzheap_tseg_at(si);
            }
         }
      }
   }
   /* Move rightmost object pointer as far left as possible */
   for(int16_t i = (vm.heap.rtobj - vm.heap.data) / sizeof(bbzobj_t) - 1;
       i >= RESERVED_ACTREC_MAX;
       --i)
      if(!bbzheap_obj_isvalid(*bbzheap_obj_at(i)))
         vm.heap.rtobj -= sizeof(bbzobj_t);
      else
         break;
   /* Move leftmost table segment pointer as far right as possible */
   for(int16_t i = (vm.heap.data + BBZHEAP_SIZE - vm.heap.ltseg) / sizeof(bbzheap_tseg_t) - 1;
       i >= 0;
       --i) {
      if(!bbzheap_tseg_isvalid(*bbzheap_tseg_at(i)))
         vm.heap.ltseg += sizeof(bbzheap_tseg_t);
      else
         break;
   }
}

/****************************************/
/****************************************/
