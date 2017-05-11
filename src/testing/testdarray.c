#include <bittybuzz/bbzdarray.h>
#include <stdio.h>
#include <inttypes.h>

static const char* bbztype_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata", "native closure" };

#define obj_isvalid(x) ((x).o.mdata & 0x10)

void bbzdarray_print(bbzheap_t* h, uint16_t d) {
   int size = bbzdarray_size(h, d);
   printf("Size: %d\n", size);
   printf("Elements: {");
   /* Prints in the format: {arrayPos: type<heapPos:[intValue]>, ...}*/
   uint16_t v;
   for (int i = 0; i < size-1; ++i) {
      bbzdarray_get(h, d, i, &v);
      printf("%d: %s<%d:[%d]>, ",i,bbztype_desc[bbztype(*bbzheap_obj_at(h, v))],v,((bbzint_t*)bbzheap_obj_at(h, v))->value);
   }
   if (size > 0) {
      bbzdarray_get(h, d, size-1, &v);
      printf("%d: %s<%d:[%d]>",size-1,bbztype_desc[bbztype(*bbzheap_obj_at(h, v))],v,((bbzint_t*)bbzheap_obj_at(h, v))->value);
   }
   printf("}\n");
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(h, d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(h, si);
   printf("aseg list: [%d:(%04x;%s)",si,bbzheap_aseg_next_get(sd),bbzheap_aseg_hasnext(sd)?"yes":"no");
   while(bbzheap_aseg_hasnext(sd)) {
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(h, si);
      //if (!bbzheap_aseg_isvalid(*sd) || !bbzheap_aseg_elem_isvalid(sd->values[0])) break;
      printf(", %d:(%04x;%s)",si,bbzheap_aseg_next_get(sd),bbzheap_aseg_hasnext(sd)?"yes":"no");
   }
   printf("]\n");
}

void bbzheap_print(bbzheap_t* h) {
   /* Object-related stuff */
   int objimax = (h->rtobj - h->data) / sizeof(bbzobj_t);
   printf("Max object index: %d\n", objimax);
   int objnum = 0;
   for(int i = 0; i < objimax; ++i)
      if(obj_isvalid(*bbzheap_obj_at(h, i))) ++objnum;
   printf("Valid objects: %d\n", objnum);
   for(int i = 0; i < objimax; ++i)
      if(obj_isvalid(*bbzheap_obj_at(h, i))) {
         printf("\t#%d: [%s]", i, bbztype_desc[bbztype(*bbzheap_obj_at(h, i))]);
         switch(bbztype(*bbzheap_obj_at(h, i))) {
            case BBZTYPE_NIL:
               break;
            case BBZTYPE_INT:
               printf(" %d", bbzheap_obj_at(h, i)->i.value);
               break;
            case BBZTYPE_FLOAT:
               printf(" %f", bbzfloat_tofloat(bbzheap_obj_at(h, i)->f.value));
               break;
            case BBZTYPE_TABLE:
               printf(" %" PRIu16, bbzheap_obj_at(h, i)->t.value);
               break;
         }
         printf("\n");
      }
   /* Segment-related stuff */
   int tsegimax = (h->data + BBZHEAP_SIZE - h->ltseg) / sizeof(bbzheap_tseg_t);
   printf("Max table segment index: %d\n", tsegimax);
   int tsegnum = 0;
   for(int i = 0; i < tsegimax; ++i)
      if(bbzheap_tseg_isvalid(*bbzheap_tseg_at(h, i))) ++tsegnum;
   printf("Valid table segments: %d\n", tsegnum);
   bbzheap_tseg_t* seg;
   for(int i = 0; i < tsegimax; ++i) {
      seg = bbzheap_tseg_at(h, i);
      if(bbzheap_tseg_isvalid(*seg)) {
         printf("\t#%d: {", i);
         for(int j = 0; j < BBZHEAP_ELEMS_PER_TSEG; ++j)
            if(bbzheap_tseg_elem_isvalid(seg->keys[j]))
               printf(" (%d,%d)",
                      bbzheap_tseg_elem_get(seg->keys[j]),
                      bbzheap_tseg_elem_get(seg->values[j]));
         printf(" /next=%x }\n", bbzheap_tseg_next_get(seg));
      }
   }
   printf("\n");
}

int main() {
   bbzheap_t heap;
   bbzheap_clear(&heap);
   
   printf("+=-=-=-=-=-= bbzdarray_new =-=-=-=-=-=+\n");
   uint16_t darray;
   bbzdarray_new(&heap, &darray);
   bbzdarray_print(&heap, darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_push =-=-=-=-=-=+\n");
   uint16_t o;
   bbzheap_obj_alloc(&heap, BBZTYPE_INT, &o);
   bbzint_t* io = (bbzint_t*)bbzheap_obj_at(&heap, o);
   io->value = 10;
   bbzdarray_push(&heap, darray, o);
   bbzdarray_print(&heap, darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_find =-=-=-=-=-=+\n");
   int pos = bbzdarray_find(&heap, darray, bbztype_cmp, o);
   printf("Position of integer %s<%d:[%d]> : %d\n", bbztype_desc[bbztype(*bbzheap_obj_at(&heap, o))],o,((bbzint_t*)bbzheap_obj_at(&heap, o))->value, pos);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_set =-=-=-=-=-=+\n");
   uint16_t o2;
   bbzheap_obj_alloc(&heap, BBZTYPE_INT, &o2);
   bbzint_t* io2 = (bbzint_t*)bbzheap_obj_at(&heap, o2);
   io2->value = 255;
   bbzdarray_set(&heap, darray, 0, o2);
   bbzdarray_print(&heap, darray);
   
   printf("+=-= Value changed in the heap =-=+\n");
   io2->value = 15;
   bbzdarray_print(&heap, darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= [15x] bbzdarray_push =-=-=-=-=-=+\n");
   uint16_t o3;
   bbzint_t* io3;
   for (int i = 0; i < 15; ++i) {
      bbzheap_obj_alloc(&heap, BBZTYPE_INT, &o3);
      io3 = (bbzint_t*)bbzheap_obj_at(&heap, o3);
      io3->value = i;
      bbzdarray_push(&heap, darray, o3);
   }
   bbzdarray_print(&heap, darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= [7x] bbzdarray_pop =-=-=-=-=-=+\n");
   for (int i = 0; i < 7; ++i) {
      bbzdarray_pop(&heap, darray);
      fflush(stdout);
   }
   bbzdarray_print(&heap, darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_clear =-=-=-=-=-=+\n");
   bbzdarray_clear(&heap, darray);
   bbzdarray_print(&heap, darray);
   printf("\n");
   
   uint16_t stack[] = {darray};
   bbzheap_gc(&heap, stack, 1);
   
   printf("+=-=-=-=-=-= bbzdarray_clone =-=-=-=-=-=+\n");
   for (int i = 0; i < 22; ++i) {
      bbzheap_obj_alloc(&heap, BBZTYPE_INT, &o3);
      io3 = (bbzint_t*)bbzheap_obj_at(&heap, o3);
      io3->value = i;
      bbzdarray_push(&heap, darray, o3);
   }
   uint16_t darray2 = darray;
   bbzdarray_clone(&heap, darray, &darray2);
   bbzdarray_print(&heap, darray);
   printf("\n");
   bbzdarray_print(&heap, darray2);
   printf("\n");
   
   bbzheap_print(&heap);
   
   return 0;
}
