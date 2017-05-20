#include <bittybuzz/bbztable.h>
#include <stdio.h>
#include <inttypes.h>

static const char* bbztype_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata" };

#define obj_isvalid(x) ((x).o.mdata & 0x10)

void bbzheap_print() {
   /* Object-related stuff */
   int objimax = (vm.heap.rtobj - vm.heap.data) / sizeof(bbzobj_t);
   printf("Max object index: %d\n", objimax);
   int objnum = 0;
   for(int i = 0; i < objimax; ++i)
      if(obj_isvalid(*bbzheap_obj_at(i))) ++objnum;
   printf("Valid objects: %d\n", objnum);
   for(int i = 0; i < objimax; ++i)
      if(obj_isvalid(*bbzheap_obj_at(i))) {
         printf("\t#%d: [%s]", i, bbztype_desc[bbztype(*bbzheap_obj_at(i))]);
         switch(bbztype(*bbzheap_obj_at(i))) {
            case BBZTYPE_NIL:
               break;
            case BBZTYPE_INT:
               printf(" %d", bbzheap_obj_at(i)->i.value);
               break;
            case BBZTYPE_FLOAT:
               printf(" %f", bbzfloat_tofloat(bbzheap_obj_at(i)->f.value));
               break;
            case BBZTYPE_TABLE:
               printf(" %" PRIu16, bbzheap_obj_at(i)->t.value);
               break;
         }
         printf("\n");
      }
   /* Segment-related stuff */
   int tsegimax = (vm.heap.data + BBZHEAP_SIZE - vm.heap.ltseg) / sizeof(bbzheap_tseg_t);
   printf("Max table segment index: %d\n", tsegimax);
   int tsegnum = 0;
   for(int i = 0; i < tsegimax; ++i)
      if(bbzheap_tseg_isvalid(*bbzheap_tseg_at(i))) ++tsegnum;
   printf("Valid table segments: %d\n", tsegnum);
   bbzheap_tseg_t* seg;
   for(int i = 0; i < tsegimax; ++i) {
      seg = bbzheap_tseg_at(i);
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

bbzvm_t vm; // Declaration for extern resolution

int main() {
   bbzheap_clear();
   bbzheap_print();
   uint16_t o;
   for(int i = 0; i < 5; ++i) {
      if(bbzheap_obj_alloc(BBZTYPE_INT, &o))
         printf("Allocated int object at position %" PRIu16 "\n", o);
      else {
         printf("Allocation error\n");
         break;
      }
      bbzheap_obj_at(o)->i.value = i;
      bbzheap_print();
   }
   printf("Garbage collection\n");
   uint16_t stack1[5] = { 1, 3, 2, 0, 0 };
   bbzheap_gc(stack1, 3);
   bbzheap_print();
   for(int i = 0; i < 3; ++i) {
      if(bbzheap_obj_alloc(BBZTYPE_FLOAT, &o))
         printf("Allocated float object at position %" PRIu16 "\n", o);
      else {
         printf("Allocation error\n");
         break;
      }
      bbzheap_obj_at(o)->f.value = bbzfloat_fromfloat(i);
      bbzheap_print();
   }
   //uint16_t s1;
   for(int i = 0; i < 3; ++i) {
      if(bbzheap_obj_alloc(BBZTYPE_TABLE, &o)) {
         printf("Allocated table object at position %" PRIu16 "\n", o);
         printf("Allocated table segment at position %" PRIu16 "\n", bbzheap_obj_at(o)->t.value);
      }
      else {
         printf("Allocation error\n");
         break;
      }/*
      if(bbzheap_tseg_alloc(&s1)) {
         printf("Allocated table segment at position %" PRIu16 "\n", s1);
         bbzheap_obj_at(o)->t.value = s1;
      }
      else {
         printf("Allocation error\n");
         break;
      }*/
      bbzheap_print();
   }   
   printf("Garbage collection\n");
   uint16_t stack2[8] = { 0, 1, 2, 3, 4, 5, 7, 8 };
   bbzheap_gc(stack2, 8);
   bbzheap_print();
   for(int i = 0; i < 5; ++i) {
      for(int j = 0; j < 2; ++j) {
         printf("Adding (#%d, #%d) to the table\n", i, (i+1));
         if(!bbztable_set(7, i, (i+1)))
            printf("Error inserting (#%d, #%d) in table\n", i, (i+1));
         else
            bbzheap_print();
      }
   }
   int sz = bbztable_size(7);
   printf("Table #7 size = %d\n", sz);
   sz = bbztable_size(8);
   printf("Table #8 size = %d\n", sz);
   printf("Garbage collection\n");
   uint16_t stack3[2] = { 7, 8 };
   bbzheap_gc(stack3, 2);
   bbzheap_print();
   if(bbzheap_obj_alloc(BBZTYPE_NIL, &o))
      printf("Allocated nil object at position %" PRIu16 "\n", o);
   else {
      printf("Allocation error\n");
   }
   for(int i = 0; i < 4; ++i) {
      printf("Adding (#%d, #%d) to the table\n", i, o);
      if(!bbztable_set(7, i, o))
         printf("Error inserting (#%d, #%d) in table\n", i, o);
      else
         bbzheap_print();
   }
   printf("Garbage collection\n");
   bbzheap_gc(stack3, 2);
   bbzheap_print();
   return 0;
}
