#include <bittybuzz/bbzvm.h>

void foreach_test_fun(bbzheap_idx_t key, bbzheap_idx_t value, void* params) {
    RM_UNUSED_WARN(key);
    RM_UNUSED_WARN(value);
    uint8_t* num_calls_foreach = (uint8_t*)params;
    ++(*num_calls_foreach);
}


int main() {
    bbzvm_t vmObj;
    vm = &vmObj;

    bbzvm_construct(0);
    bbzheap_clear();
    bbzheap_print();
    uint16_t o;
    for(int i = 0; i < 5; ++i) {
        if(bbzheap_obj_alloc(BBZTYPE_INT, &o))
            printf("[testheap] Allocated int object at position %" PRIu16 "\n", o);
        else {
            printf("[testheap] Allocation error\n");
            break;
        }
        bbzheap_obj_at(o)->i.value = i;
        bbzheap_print();
    }
    printf("[testheap] Garbage collection\n");
    uint16_t stack1[5] = { RESERVED_ACTREC_MAX + 1, RESERVED_ACTREC_MAX + 3, RESERVED_ACTREC_MAX + 2, 0, 0 };
    bbzheap_gc(stack1, 3);
    bbzheap_print();
    for(int i = 0; i < 3; ++i) {
        if(bbzheap_obj_alloc(BBZTYPE_FLOAT, &o))
            printf("[testheap] Allocated float object at position %" PRIu16 "\n", o);
        else {
            printf("[testheap] Allocation error\n");
            break;
        }
        bbzheap_obj_at(o)->f.value = bbzfloat_fromfloat(i);
        bbzheap_print();
    }
    //uint16_t s1;
    for(int i = 0; i < 3; ++i) {
        if(bbzheap_obj_alloc(BBZTYPE_TABLE, &o)) {
            printf("[testheap] Allocated table object at position %" PRIu16 "\n", o);
            printf("[testheap] Allocated table segment at position %" PRIu16 "\n", bbzheap_obj_at(o)->t.value);
        }
        else {
            printf("[testheap] Allocation error\n");
            break;
        }/*
        if(bbzheap_tseg_alloc(&s1)) {
            printf("[testheap] Allocated table segment at position %" PRIu16 "\n", s1);
            bbzheap_obj_at(o)->t.value = s1;
        }
        else {
            printf("[testheap] Allocation error\n");
            break;
        }*/
        bbzheap_print();
    }
    printf("[testheap] Garbage collection\n");
    uint16_t stack2[8] = { RESERVED_ACTREC_MAX + 0, RESERVED_ACTREC_MAX + 1, RESERVED_ACTREC_MAX + 2, RESERVED_ACTREC_MAX + 3,
                           RESERVED_ACTREC_MAX + 4, RESERVED_ACTREC_MAX + 5, RESERVED_ACTREC_MAX + 7, RESERVED_ACTREC_MAX + 8 };
    bbzheap_gc(stack2, 8);
    bbzheap_print();
    for(int i = 0; i < 5; ++i) {
        for(int j = 0; j < 2; ++j) {
            printf("[testheap] Adding (#%d, #%d) to the table\n", i, (i+1));
            if(!bbztable_set(7, i, (i+1)))
                printf("[testheap] Error inserting (#%d, #%d) in table\n", i, (i+1));
            else
                bbzheap_print();
        }
    }
    int sz = bbztable_size(7);
    printf("[testheap] Table #7 size = %d\n", sz);
    sz = bbztable_size(8);
    printf("[testheap] Table #8 size = %d\n", sz);
    printf("[testheap] Garbage collection\n");
    uint16_t stack3[2] = { RESERVED_ACTREC_MAX + 7, RESERVED_ACTREC_MAX + 8 };
    bbzheap_gc(stack3, 2);
    bbzheap_print();
    if(bbzheap_obj_alloc(BBZTYPE_NIL, &o))
        printf("[testheap] Allocated nil object at position %" PRIu16 "\n", o);
    else {
        printf("[testheap] Allocation error\n");
    }
    for(int i = 0; i < 4; ++i) {
        printf("[testheap] Adding (#%d, #%d) to the table\n", i, o);
        if(!bbztable_set(7, i, o))
            printf("[testheap] Error inserting (#%d, #%d) in table\n", i, o);
        else
            bbzheap_print();
    }
    printf("[testheap] Garbage collection\n");
    bbzheap_gc(stack3, 2);
    bbzheap_print();

    // Test foreach
    {
        printf("[testheap] Foreach on an empty table.\n");
        bbzvm_pusht();
        bbzheap_idx_t t = bbzvm_stack_at(0);

        uint8_t num_calls_foreach = 0;
        bbztable_foreach(t, foreach_test_fun, &num_calls_foreach);
        if (num_calls_foreach == 0) {
            printf("[testheap] OK\n\n");
        }
        else {
            printf("[testheap] Foreach called when it was not expected to.\n\n");
        }

        num_calls_foreach = 0;
        printf("[testheap] Foreach on a non-empty table.\n");
        for (uint8_t i = 0; i < 10; ++i) {
            bbzvm_pushi(2*i);
            bbzvm_pushi(2*i + 1);
            bbzheap_idx_t key   = bbzvm_stack_at(1);
            bbzheap_idx_t value = bbzvm_stack_at(0);
            bbzvm_pop();
            bbzvm_pop();
            printf("[testheap] Adding (%d, %d) to the table\n", 2*i, 2*i+1);
            bbztable_set(t, key, value);
        }
        bbzheap_print();
        bbztable_foreach(t, foreach_test_fun, &num_calls_foreach);

        if (num_calls_foreach == 10) {
            printf("[testheap] OK\n\n");
        }
        else {
            printf("[testheap] Foreach called wrong number of times (%d) instead of 10.\n\n", num_calls_foreach);
        }

        printf("[testheap] Garbage collection\n");
        bbzheap_gc(stack3, 2);
        bbzheap_print();
    }

    return 0;
}
