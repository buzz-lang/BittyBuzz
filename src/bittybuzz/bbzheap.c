#include "bbzheap.h"
#include <stdio.h>

/****************************************/
/****************************************/

#define obj_makevalid(x)   (x).o.mdata |= 0x10
#define obj_makeinvalid(x) (x).o.mdata &= 0xEF

#define tseg_makevalid(s)   (s).mdata = 0xFFFF // Make the segment valid AND set next to -1
#define tseg_makeinvalid(s) (s).mdata &= 0x7FFF

#define gc_hasmark(x) ((x).o.mdata & 0x08)
#define gc_mark(x)    (x).o.mdata |= 0x08
#define gc_unmark(x)  (x).o.mdata &= 0xF7

/****************************************/
/****************************************/

void bbzheap_clear(bbzheap_t* h) {
   h->rtobj = h->data;
   h->ltseg = h->data + BBZHEAP_SIZE;
   for(int i = BBZHEAP_SIZE-1; i >= 0; --i) h->data[i] = 0;
}

/****************************************/
/****************************************/

int bbzheap_obj_alloc(bbzheap_t* h,
                      int t,
                      uint16_t* o) {
   /* Look for empty slot */
   for(int i = (h->rtobj - h->data) / sizeof(bbzobj_t) - 1;
       i >= 0;
       --i)
      if(!bbzheap_obj_isvalid(*bbzheap_obj_at(h, i))) {
         /* Empty slot found */
         bbzobj_t* x = bbzheap_obj_at(h, i);
         /* Set valid bit and type */
         obj_makevalid(*x);
         bbztype_cast(*x, t);
         /* Set result */
         *o = i;
         /* Success */
         return 1;
      }
   /* No empty slot found, must create a new one */
   /* ...but first, make sure there is room */
   if(h->rtobj + sizeof(bbzobj_t) > h->ltseg) return 0;
   /* Set result */
   *o = (h->rtobj - h->data) / sizeof(bbzobj_t);
   /* Set valid bit and type */
   obj_makevalid(*((bbzobj_t*)h->rtobj));
   bbztype_cast(*((bbzobj_t*)h->rtobj), t);
   /* Make room */
   h->rtobj += sizeof(bbzobj_t);
   return 1;
}

/****************************************/
/****************************************/

int bbzheap_tseg_alloc(bbzheap_t* h,
                       uint16_t* s) {
   /* Look for empty slot */
   for(int i = (h->data + BBZHEAP_SIZE - h->ltseg) / sizeof(bbzheap_tseg_t) - 1;
       i >= 0;
       --i)
      if(!bbzheap_tseg_isvalid(*bbzheap_tseg_at(h, i))) {
         /* Empty slot found */
         bbzheap_tseg_t* x = bbzheap_tseg_at(h, i);
         /* Set valid bit of segment and -1 index for next */
         tseg_makevalid(*x);
         /* Invalidate keys and values */
         for(int i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            x->keys[i] = 0;
            x->values[i] = 0;
         }
         /* Set result */
         *s = i;
         /* Success */
         return 1;
      }
   /* Make sure there is room */
   if(h->ltseg - sizeof(bbzheap_tseg_t) < h->rtobj) return 0;
   /* Set result */
   *s = (h->data + BBZHEAP_SIZE - h->ltseg) / sizeof(bbzheap_tseg_t);
   /* Update pointer to leftmost invalid segment */
   h->ltseg -= sizeof(bbzheap_tseg_t);
   /* Set valid bit of segment and zero index for next */
   tseg_makevalid(*(bbzheap_tseg_t*)(h->ltseg));
   /* Invalidate keys and values */
   for(int i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
      ((bbzheap_tseg_t*)h->ltseg)->keys[i] = 0;
      ((bbzheap_tseg_t*)h->ltseg)->values[i] = 0;
   }
   return 1;
}

/****************************************/
/****************************************/

void bbzheap_gc(bbzheap_t* h,
                uint16_t* st,
                int sz) {
   /* Set all gc bits to zero */
   for(int i = (h->rtobj - h->data) / sizeof(bbzobj_t) - 1; i >= 0; --i)
      gc_unmark(*bbzheap_obj_at(h, i));
   /* Go through the stack and set the gc bit of valid variables */
   for(int i = 0; i < sz; ++i) {
      /* Mark gc bit */
      gc_mark(*bbzheap_obj_at(h, st[i]));
      /* If it's a table, go through it and mark all associated objects */
      if(bbztype_istable(*bbzheap_obj_at(h, st[i]))) {
         /* Segment index in heap */
         uint16_t si = bbzheap_obj_at(h, st[i])->t.value;
         if(bbztype_isdarray(*bbzheap_obj_at(h, st[i]))) {
            /* Actual segment data in heap */
            bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
            /* Go through the segments */
            while(1) {
               for(int j = 0; j < 2*BBZHEAP_ELEMS_PER_TSEG; ++j)
                  if(bbzheap_aseg_elem_isvalid(sd->values[j])) {
                     gc_mark(*bbzheap_obj_at(h, bbzheap_aseg_elem_get(sd->values[j])));
                  }
               if(!bbzheap_aseg_hasnext(sd)) break;
               si = bbzheap_aseg_next_get(sd);
               sd = bbzheap_aseg_at(h, si);
            }
         }
         else {
            /* Actual segment data in heap */
            bbzheap_tseg_t* sd = bbzheap_tseg_at(h, si);
            /* Go through the segments */
            while(1) {
               for(int j = 0; j < BBZHEAP_ELEMS_PER_TSEG; ++j)
                  if(bbzheap_tseg_elem_isvalid(sd->keys[j])) {
                     gc_mark(*bbzheap_obj_at(h, bbzheap_tseg_elem_get(sd->keys[j])));
                     gc_mark(*bbzheap_obj_at(h, bbzheap_tseg_elem_get(sd->values[j])));
                  }
               if(!bbzheap_tseg_hasnext(sd)) break;
               si = bbzheap_tseg_next_get(sd);
               sd = bbzheap_tseg_at(h, si);
            }
         }
      }
   }
   /* Go through the objects; invalidate those with 0 gc bit */
   for(int i = (h->rtobj - h->data) / sizeof(bbzobj_t) - 1; i >= 0; --i) {
      if(!gc_hasmark(*bbzheap_obj_at(h, i))) {
         /* Invalidate object */
         obj_makeinvalid(*bbzheap_obj_at(h, i));
         /* If it's a table, invalidate its segments too */
         if(bbztype_istable(*bbzheap_obj_at(h, i))) {
            /* Segment index in heap */
            uint16_t si = bbzheap_obj_at(h, i)->t.value;
            /* Actual segment data in heap */
            bbzheap_tseg_t* sd = bbzheap_tseg_at(h, si);
            /* Go through the segments and invalidate them all */
            while(1) {
               tseg_makeinvalid(*sd);
               if(!bbzheap_tseg_hasnext(sd)) break;
               si = bbzheap_tseg_next_get(sd);
               sd = bbzheap_tseg_at(h, si);
            }
         }
      }
   }
   /* Move rightmost object pointer as far left as possible */
   for(int i = (h->rtobj - h->data) / sizeof(bbzobj_t) - 1;
       i >= 0;
       --i)
      if(!bbzheap_obj_isvalid(*bbzheap_obj_at(h, i)))
         h->rtobj -= sizeof(bbzobj_t);
      else
         break;
   /* Move leftmost table segment pointer as far right as possible */
   for(int i = (h->data + BBZHEAP_SIZE - h->ltseg) / sizeof(bbzheap_tseg_t) - 1;
       i >= 0;
       --i) {
      if(!bbzheap_tseg_isvalid(*bbzheap_tseg_at(h, i)))
         h->ltseg += sizeof(bbzheap_tseg_t);
      else
         break;
   }
}

/****************************************/
/****************************************/
