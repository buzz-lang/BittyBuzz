#include <bittybuzz/bbztable.h>
#include <stdio.h>
#include <inttypes.h>

static const char* bbztype_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata" };

#define obj_isvalid(x) ((x).o.mdata & 0x10)

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
   bbzheap_print(&heap);
   uint16_t o;
   for(int i = 0; i < 5; ++i) {
      if(bbzheap_obj_alloc(&heap, BBZTYPE_INT, &o))
         printf("Allocated int object at position %" PRIu16 "\n", o);
      else {
         printf("Allocation error\n");
         break;
      }
      bbzheap_obj_at(&heap, o)->i.value = i;
      bbzheap_print(&heap);
   }
   printf("Garbage collection\n");
   uint16_t stack1[5] = { 1, 3, 2, 0, 0 };
   bbzheap_gc(&heap, stack1, 3);
   bbzheap_print(&heap);
   for(int i = 0; i < 3; ++i) {
      if(bbzheap_obj_alloc(&heap, BBZTYPE_FLOAT, &o))
         printf("Allocated float object at position %" PRIu16 "\n", o);
      else {
         printf("Allocation error\n");
         break;
      }
      bbzheap_obj_at(&heap, o)->f.value = bbzfloat_fromfloat(i);
      bbzheap_print(&heap);
   }
   //uint16_t s1;
   for(int i = 0; i < 3; ++i) {
      if(bbzheap_obj_alloc(&heap, BBZTYPE_TABLE, &o)) {
         printf("Allocated table object at position %" PRIu16 "\n", o);
         printf("Allocated table segment at position %" PRIu16 "\n", bbzheap_obj_at(&heap, o)->t.value);
      }
      else {
         printf("Allocation error\n");
         break;
      }/*
      if(bbzheap_tseg_alloc(&heap, &s1)) {
         printf("Allocated table segment at position %" PRIu16 "\n", s1);
         bbzheap_obj_at(&heap, o)->t.value = s1;
      }
      else {
         printf("Allocation error\n");
         break;
      }*/
      bbzheap_print(&heap);
   }   
   printf("Garbage collection\n");
   uint16_t stack2[8] = { 0, 1, 2, 3, 4, 5, 7, 8 };
   bbzheap_gc(&heap, stack2, 8);
   bbzheap_print(&heap);
   for(int i = 0; i < 5; ++i) {
      for(int j = 0; j < 2; ++j) {
         printf("Adding (#%d, #%d) to the table\n", i, (i+1));
         if(!bbztable_set(&heap, 7, i, (i+1)))
            printf("Error inserting (#%d, #%d) in table\n", i, (i+1));
         else
            bbzheap_print(&heap);
      }
   }
   int sz = bbztable_size(&heap, 7);
   printf("Table #7 size = %d\n", sz);
   sz = bbztable_size(&heap, 8);
   printf("Table #8 size = %d\n", sz);
   printf("Garbage collection\n");
   uint16_t stack3[2] = { 7, 8 };
   bbzheap_gc(&heap, stack3, 2);
   bbzheap_print(&heap);
   if(bbzheap_obj_alloc(&heap, BBZTYPE_NIL, &o))
      printf("Allocated nil object at position %" PRIu16 "\n", o);
   else {
      printf("Allocation error\n");
   }
   for(int i = 0; i < 4; ++i) {
      printf("Adding (#%d, #%d) to the table\n", i, o);
      if(!bbztable_set(&heap, 7, i, o))
         printf("Error inserting (#%d, #%d) in table\n", i, o);
      else
         bbzheap_print(&heap);
   }
   printf("Garbage collection\n");
   bbzheap_gc(&heap, stack3, 2);
   bbzheap_print(&heap);
   return 0;
}
