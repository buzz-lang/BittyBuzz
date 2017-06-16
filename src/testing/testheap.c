#include <bittybuzz/bbztable.h>
#include <stdio.h>
#include <inttypes.h>

void foreach_test_fun(bbzheap_idx_t key, bbzheap_idx_t value, void* params) {
    uint8_t* num_calls_foreach = (uint8_t*)params;
    ++(*num_calls_foreach);
}


int main() {
    bbzvm_t vmObj;
    vm = &vmObj;

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

    // Test foreach
    {
        printf("Foreach on an empty table.\n");
        bbzvm_pusht();
        bbzheap_idx_t t = bbzvm_stack_at(0);

        uint8_t num_calls_foreach = 0;
        bbztable_foreach(t, foreach_test_fun, &num_calls_foreach);
        if (num_calls_foreach == 0) {
            printf("OK\n\n");
        }
        else {
            printf("Foreach called when it was not expected to.\n\n");
        }
        
        num_calls_foreach = 0;
        printf("Foreach on a non-empty table.\n");
        for (uint8_t i = 0; i < 10; ++i) {
            bbzvm_pushi(2*i);
            bbzvm_pushi(2*i + 1);
            bbzheap_idx_t key   = bbzvm_stack_at(1);
            bbzheap_idx_t value = bbzvm_stack_at(0);
            bbzvm_pop();
            bbzvm_pop();
            printf("Adding (%d, %d) to the table\n", 2*i, 2*i+1);
            bbztable_set(t, key, value);
        }
        bbzheap_print();
        bbztable_foreach(t, foreach_test_fun, &num_calls_foreach);

        if (num_calls_foreach == 10) {
            printf("OK\n\n");
        }
        else {
            printf("Foreach called wrong number of times (%d) instead of 10.\n\n", num_calls_foreach);
        }
        
        printf("Garbage collection\n");
        bbzheap_gc(stack3, 2);
        bbzheap_print();
    }

    return 0;
}
