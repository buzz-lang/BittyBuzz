#include <bittybuzz/bbzobjringbuf.h>
#include <stdio.h>

/*
static const char* bbztype_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata" };

void bbzobjringbuf_print(const bbzringbuf_t* ring) {
   printf("Ring buffer size = %u\n", bbzobjringbuf_size(*ring));
   for(uint8_t i = 0; i < bbzobjringbuf_size(*ring); ++i) {
      printf("  #%u -> [%s]", i, bbztype_desc[bbztype(bbzobjringbuf_at(*ring, i))]);
      switch(bbztype(bbzobjringbuf_rawat(*ring, i))) {
         case BBZTYPE_NIL:
            break;
         case BBZTYPE_INT:
            printf(" %d", bbzobjringbuf_rawat(*ring, i).i.value);
            break;
      }
      printf("\n");
   }
   printf("\n");
}
*/

int main() {
/*
   bbzobj_t linear[10];
   bbzobjringbuf_t ring;
   bbzobjringbuf_new(ring, linear, 10);
   bbzobjringbuf_print(&ring);
   for(int i = 0; i < 11; ++i) {
      // bbzobjringbuf_append_nil(ring);
      // bbzobjringbuf_print(&ring);
      bbzobjringbuf_append_int(ring, i);
      bbzobjringbuf_print(&ring);
   }
   return 0;
*/
}
