#include <bittybuzz/bbzdarray.h>
#include <bittybuzz/bbzvm.h>
#include <stdio.h>
#include <inttypes.h>

static const char* bbztype_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata", "native closure" };

void bbzdarray_print(bbzheap_idx_t d) {
   uint16_t size = bbzdarray_size(d);
   printf("Size: %d\n", size);
   printf("Elements: [\n");
   /* Prints in the format: [arrayPos: type<heapPos:{intValue}>, ...]*/
   uint16_t v;
   for (uint16_t i = 0; i < size-1; ++i) {
      bbzdarray_get(d, i, &v);
      printf("\t#%d: %s<%d:{%d}>,\n",i,bbztype_desc[bbztype(*bbzheap_obj_at(v))],v,((bbzint_t*)bbzheap_obj_at(v))->value);
   }
   if (size > 0) {
      bbzdarray_get(d, size-1, &v);
      printf("\t#%d: %s<%d:{%d}>\n",size-1,bbztype_desc[bbztype(*bbzheap_obj_at(v))],v,((bbzint_t*)bbzheap_obj_at(v))->value);
   }
   printf("]\n");
   bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(d);
   uint16_t si = da->value;
   bbzheap_aseg_t* sd = bbzheap_aseg_at(si);
   printf("aseg list: [%d:(%04x;%s)",si,bbzheap_aseg_next_get(sd),bbzheap_aseg_hasnext(sd)?"yes":"no");
   while(bbzheap_aseg_hasnext(sd)) {
      si = bbzheap_aseg_next_get(sd);
      sd = bbzheap_aseg_at(si);
      if (!bbzheap_aseg_isvalid(*sd) || !bbzheap_aseg_elem_isvalid(sd->values[0])) break;
      printf(", %d:(%04x;%s)",si,bbzheap_aseg_next_get(sd),bbzheap_aseg_hasnext(sd)?"yes":"no");
   }
   printf("]\n");
}

void foreach(bbzheap_idx_t darray, bbzheap_idx_t pos, void* params) {
   ((bbzint_t*)bbzheap_obj_at(pos))->value += 20;
}

int main() {
   bbzvm_t vmObj;
   vm = &vmObj;

   bbzheap_clear();
   
   printf("+=-=-=-=-=-= bbzdarray_new =-=-=-=-=-=+\n");
   uint16_t darray;
   bbzdarray_new(&darray);
   bbzdarray_print(darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_push =-=-=-=-=-=+\n");
   uint16_t o;
   bbzheap_obj_alloc(BBZTYPE_INT, &o);
   bbzint_t* io = (bbzint_t*)bbzheap_obj_at(o);
   io->value = 10;
   bbzdarray_push(darray, o);
   bbzdarray_print(darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_find =-=-=-=-=-=+\n");
   int pos = bbzdarray_find(darray, bbztype_cmp, o);
   printf("Position of integer %s<%d:[%d]> : %d\n", bbztype_desc[bbztype(*bbzheap_obj_at(o))],o,((bbzint_t*)bbzheap_obj_at(o))->value, pos);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_set =-=-=-=-=-=+\n");
   uint16_t o2;
   bbzheap_obj_alloc(BBZTYPE_INT, &o2);
   bbzint_t* io2 = (bbzint_t*)bbzheap_obj_at(o2);
   io2->value = 255;
   bbzdarray_set(darray, 0, o2);
   bbzdarray_print(darray);
   
   printf("+=-= Value changed in the heap =-=+\n");
   io2->value = 15;
   bbzdarray_print(darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= [15x] bbzdarray_push =-=-=-=-=-=+\n");
   uint16_t o3;
   bbzint_t* io3;
   for (int i = 0; i < 15; ++i) {
      bbzheap_obj_alloc(BBZTYPE_INT, &o3);
      io3 = (bbzint_t*)bbzheap_obj_at(o3);
      io3->value = i;
      bbzdarray_push(darray, o3);
   }
   bbzdarray_print(darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= [7x] bbzdarray_pop =-=-=-=-=-=+\n");
   for (int i = 0; i < 7; ++i) {
      bbzdarray_pop(darray);
      fflush(stdout);
   }
   bbzdarray_print(darray);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_clear =-=-=-=-=-=+\n");
   bbzdarray_clear(darray);
   bbzdarray_print(darray);
   printf("\n");
   
   uint16_t stack[] = {darray};
   bbzheap_gc(stack, 1);
   
   printf("+=-=-=-=-=-= bbzdarray_clone =-=-=-=-=-=+\n");
   for (int i = 0; i < 22; ++i) {
      bbzheap_obj_alloc(BBZTYPE_INT, &o3);
      io3 = (bbzint_t*)bbzheap_obj_at(o3);
      io3->value = i;
      bbzdarray_push(darray, o3);
   }
   uint16_t darray2 = darray;
   bbzdarray_clone(darray, &darray2);
   bbzdarray_print(darray);
   printf("\n");
   bbzdarray_print(darray2);
   printf("\n");
   
   printf("+=-=-=-=-=-= bbzdarray_foreach [add 20 to each value] =-=-=-=-=-=+\n");
   bbzdarray_foreach(darray, foreach, NULL);
   bbzdarray_print(darray);
   printf("\n");
   
   printf("+=-=-=-=-=-=-=-=- bbzdarray_destroy -=-=-=-=-=-=-=-=+\n");
   printf("+= (Should be different for cloned and not cloned) =+\n");
   printf("++ Heap before:\n");
   bbzheap_print();
   printf("++ Destroying arrays...\n");
   bbzdarray_destroy(darray);
   bbzdarray_destroy(darray2);
   printf("++ Heap after:\n");
   bbzheap_print();
   
   return 0;
}
