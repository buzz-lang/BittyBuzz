#include "bbztable.h"
#include "bbzvm.h"

/****************************************/
/****************************************/

uint8_t bbztable_get(bbzheap_idx_t t,
                     bbzheap_idx_t k,
                     bbzheap_idx_t* v) {
    if (!bbztype_istable(*bbzheap_obj_at(t))) return 0;
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Get segment data */
    bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
    /* Go through segments */
    while (1) {
        /* Go through valid keys in the segment */
        for (uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (bbzheap_tseg_elem_isvalid(sd->keys[i]) &&
                bbztype_cmp(bbzheap_obj_at(bbzheap_tseg_elem_get(sd->keys[i])),
                            bbzheap_obj_at(k)) == 0) {
                /* Key found */
                *v = bbzheap_tseg_elem_get(sd->values[i]);
                return 1;
            }
        }
        /* Are we done? */
        if (!bbzheap_tseg_hasnext(sd)) return 0;
        /* Get next segment */
        si = bbzheap_tseg_next_get(sd);
        sd = bbzheap_tseg_at(si);
    }
}

/****************************************/
/****************************************/

uint8_t bbztable_set(bbzheap_idx_t t,
                     bbzheap_idx_t k,
                     bbzheap_idx_t v) {
    /* Search for the given key, keeping track of first free slot */
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Get segment data */
    bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
    /* Free segment and slot */
    int16_t fseg = -1, fslot = -1;
    /* Target segment and slot */
    int16_t seg = -1, slot = -1;
    /* Go through segments */
    while (1) {
        // bbzheap_idx_t key;
        /* Go through valid keys in the segment */
        for (uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            // bbzvm_assign(&key, sd->keys + i);
            if (!bbzheap_tseg_elem_isvalid(sd->keys[i])) {
                if (fseg < 0) {
                    /* First free slot found */
                    fseg = si;
                    fslot = i;
                }
            }
            else if (bbztype_cmp(bbzheap_obj_at(bbzheap_tseg_elem_get(sd->keys[i])),
                                bbzheap_obj_at(k)) == 0) {
                /* Key found */
                seg = si;
                slot = i;
            }
        }
        /* Did we find the key? */
        if (seg >= 0) break;
        /* Are we done? */
        if (!bbzheap_tseg_hasnext(sd)) break;
        /* Get next segment */
        si = bbzheap_tseg_next_get(sd);
        sd = bbzheap_tseg_at(si);
    }
    /* When we get here, we have three possibilities:
        * 1. We found the key;
        * 2. We did not find the key, and found an empty slot;
        * 3. We did not find the key, nor found an empty slot
        * Also, sd points to the last segment visited, and si is its index.
        */
    if(seg >= 0) {
        /* 1. We found the key, change associated value */
        /* NOTE: Setting a value to nil is equivalent to erasing the element from the table */
        if(!bbztype_isnil(*bbzheap_obj_at(v)))
            bbzheap_tseg_elem_set(bbzheap_tseg_at(seg)->values[slot], v);
        else {
            /* Setting nil, erase element */
            bbzheap_tseg_at(seg)->keys[slot] = 0;
            bbzheap_tseg_at(seg)->values[slot] = 0;
            /* Is segment empty? */
            for(uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
                if(bbzheap_tseg_elem_isvalid(sd->keys[i])) return 1;
            }
            /* If we get here, it's because the segment is empty */
            /* Is it the first segment? */
            if(si == bbzheap_obj_at(t)->t.value) {
                /* Yes; is it the only segment for this table? */
                if(bbzheap_tseg_hasnext(sd)) {
                    /* No, there's more segments */
                    /* Update the table segment index */
                    bbzheap_obj_at(t)->t.value = bbzheap_tseg_next_get(sd);
                    /* Invalidate the segment */
                    sd->mdata = 0;
                }
            }
            else {
                /* No, the segment is not the first */
                /* Find the preceding segment */
                uint16_t pi = bbzheap_obj_at(t)->t.value;
                bbzheap_tseg_t* pd = bbzheap_tseg_at(pi);
                while(bbzheap_tseg_next_get(pd) != si) {
                    pi = bbzheap_tseg_next_get(pd);
                    pd = bbzheap_tseg_at(pi);
                }
                /* Set the next of the preceding to the next of current */
                bbzheap_tseg_next_set(pd, bbzheap_tseg_next_get(sd));
                /* Invalidate the current segment */
                sd->mdata = 0;
            }
        }
    }
    /* Ignore setting nil on new elements */
    else if(!bbztype_isnil(*bbzheap_obj_at(v))) {
        if(fseg >= 0) {
            /* 2. We did not find the key, and found an empty slot */
            bbzheap_tseg_elem_set(bbzheap_tseg_at(fseg)->keys[fslot], k);
            bbzheap_tseg_elem_set(bbzheap_tseg_at(fseg)->values[fslot], v);
        }
        else {
            /* 3. We did not find the key, nor an empty slot */
            /* Create a new segment */
            uint16_t s = BBZHEAP_SEG_NO_NEXT;
            if(!bbzheap_tseg_alloc(&s)) return 0;
            bbzheap_tseg_next_set(sd, s);
            /* Set key and value */
            bbzheap_tseg_elem_set(bbzheap_tseg_at(s)->keys[0], k);
            bbzheap_tseg_elem_set(bbzheap_tseg_at(s)->values[0], v);
        }
    }
    /* Success */
    return 1;
}

/****************************************/
/****************************************/

uint8_t bbztable_size(bbzheap_idx_t t) {
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Get segment data */
    bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
    /* Initialize size to zero */
    uint8_t sz = 0;
    /* Go through elements and segments */
    while(1) {
        /* Count valid keys in the segment */
        for(uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i)
            if(bbzheap_tseg_elem_isvalid(sd->keys[i])) ++sz;
        /* Are we done? */
        if(!bbzheap_tseg_hasnext(sd)) break;
        /* Get next segment */
        si = bbzheap_tseg_next_get(sd);
        sd = bbzheap_tseg_at(si);
    }
    /* Done */
    return sz;
}

/****************************************/
/****************************************/

void bbztable_foreach(bbzheap_idx_t t, bbztable_elem_funp fun, void* params) {
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Go through each segment */
    bbzheap_tseg_t* tseg;
    do {
        tseg = bbzheap_tseg_at(si);
        /* Go through each valid element in the segment */
        for (uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (bbzheap_tseg_elem_isvalid(tseg->keys[i])) {
                /* Call function */
                fun(bbzheap_tseg_elem_get(tseg->keys[i]),
                    bbzheap_tseg_elem_get(tseg->values[i]),
                    params);
            }
        }
        si = bbzheap_tseg_next_get(tseg);
    } while (si != BBZHEAP_SEG_NO_NEXT);
}

/****************************************/
/****************************************/

static void table_foreach_entry(bbzheap_idx_t key, bbzheap_idx_t value, void* params) {
    /* Cast params */
    bbzheap_idx_t* closure = params;

    // Save stack size
    uint16_t ss = bbzvm_stack_size();

    /* Push self table, closure and params (key and value) */
    bbzvm_lload(1); // Push self table
    bbzvm_push(*closure);
    bbzvm_push(key);
    bbzvm_push(value);

    /* Call closure */
    bbzvm_closure_call(2);

    // Make sure we don't return a value in the foreach function.
    bbzvm_assert_exec(bbzvm_stack_size() > ss, BBZVM_ERROR_RET);
    bbzvm_pop(); // Pop self table
}

static void table_foreach() {
    bbzvm_assert_lnum(2);

    // Get table
    bbzheap_idx_t t = bbzvm_locals_at(1);
    bbzvm_assert_type(t, BBZTYPE_TABLE);

    // Get closure
    bbzheap_idx_t c = bbzvm_locals_at(2);
    bbzvm_assert_type(c, BBZTYPE_CLOSURE);

    // Perform foreach
    bbztable_foreach(t, table_foreach_entry, &c);

    bbzvm_ret0();
}

/****************************************/
/****************************************/
/**
 * @brief Function that puts an element in a table.
 * @param[in] t the table to push at
 * @param[in] k the key at the table
 * @param[in] v the value at the table
 * @param[in] ret the value returned by the map/filter closure
 */
typedef void (*set_elem_funp)(bbzheap_idx_t t, bbzheap_idx_t k, bbzheap_idx_t v, bbzheap_idx_t ret);

/**
 * @brief Parameter struct to pass to the element-wise function of 'map'
 * and 'filter'.
 */
typedef struct PACKED table_map_base_t {
    const bbzheap_idx_t t;  /**< @brief Return table of the map. */
    const bbzheap_idx_t c;  /**< @brief Closure to call. */
    const set_elem_funp set_elem; /**< @brief Function used to set the element. */
} table_map_base_t;

static void table_map_base_entry(bbzheap_idx_t key, bbzheap_idx_t value, void* params){
    table_map_base_t* tm = params;

    // Save stack size
    uint16_t ss = bbzvm_stack_size();

    // Call closure
    bbzvm_lload(1); // Push self table
    bbzvm_push(tm->c);
    bbzvm_push(key);
    bbzvm_push(value);
    bbzvm_closure_call(2);

    // Make sure we returned a value, and get the value.
    bbzvm_assert_exec(bbzvm_stack_size() > ss, BBZVM_ERROR_RET);
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    bbzvm_pop(); // Pop return value

    // Add a value to return table.
    tm->set_elem(tm->t, key, value, ret);
}

static void table_map_base(set_elem_funp set_elem){
    bbzvm_assert_lnum(2);

    // Get table
    bbzheap_idx_t t = bbzvm_locals_at(1);
    bbzvm_assert_type(t, BBZTYPE_TABLE);

    // Get closure
    bbzheap_idx_t c = bbzvm_locals_at(2);
    bbzvm_assert_type(c, BBZTYPE_CLOSURE);

    // Make return table
    bbzvm_pusht();
    bbzheap_idx_t ret_tbl = bbzvm_stack_at(0);

    // Perform foreach
    table_map_base_t tm = { .t = ret_tbl, .c = c, .set_elem = set_elem};
    bbztable_foreach(t, table_map_base_entry, &tm);

    // Table is already stack top. Return.
    bbzvm_ret1();
}

static void table_map_set(bbzheap_idx_t t, bbzheap_idx_t k, bbzheap_idx_t v, bbzheap_idx_t ret){
    RM_UNUSED_WARN(v);
    bbztable_set(t, k, ret); // Unconditionnaly add the returned value
}

static void table_map(){
    table_map_base(table_map_set);
}

/****************************************/
/****************************************/

static void table_filter_set(bbzheap_idx_t t, bbzheap_idx_t k, bbzheap_idx_t v, bbzheap_idx_t ret){
    // Add value only if predicate is true
    if(bbztype_tobool(bbzheap_obj_at(ret))){
        bbztable_set(t, k, v); 
    }
}
static void table_filter(){
    table_map_base(table_filter_set);
}

/****************************************/
/****************************************/

/**
 * @brief Parameter struct to pass to the element-wise function of reduce
 */
typedef struct PACKED table_reduce_entry_t {
    const bbzheap_idx_t c;  /**< @brief Closure to call. */
    bbzheap_idx_t accum; /** < @brief Function used to set the element. */
} table_reduce_entry_t;

static void table_reduce_entry(bbzheap_idx_t key, bbzheap_idx_t value, void* params) {
    table_reduce_entry_t* tr = params;

    // Save stack size
    uint16_t ss = bbzvm_stack_size();

    // Call closure
    bbzvm_lload(1); // Push self table
    bbzvm_push(tr->c);
    bbzvm_push(key);
    bbzvm_push(value);
    bbzvm_push(tr->accum);
    bbzvm_closure_call(3);

    // Make sure we returned a value.
    bbzvm_assert_exec(bbzvm_stack_size() > ss, BBZVM_ERROR_RET);

    // Remove the return value and assign the accumulator the new value
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    bbzvm_pop(); // Pop return value
    tr->accum = ret;
}

static void table_reduce(){
    bbzvm_assert_lnum(3);

    // Get table
    bbzheap_idx_t t = bbzvm_locals_at(1);
    bbzvm_assert_type(t, BBZTYPE_TABLE);

    // Get closure
    bbzheap_idx_t c = bbzvm_locals_at(2);
    bbzvm_assert_type(c, BBZTYPE_CLOSURE);

    // Get accumulator
    bbzheap_idx_t a = bbzvm_locals_at(3);

    // Perform foreach and accumulate
    table_reduce_entry_t tr = {.c = c, .accum = a};
    bbztable_foreach(t, table_reduce_entry, &tr);

    // Push the accumulator as the return value
    bbzvm_push(tr.accum); 
    bbzvm_ret1();
}
/****************************************/
/****************************************/
static void table_size(){
    bbzvm_assert_lnum(1);

    // Get table
    bbzheap_idx_t t = bbzvm_locals_at(1);
    bbzvm_assert_type(t, BBZTYPE_TABLE);

    // Calculate the size
    uint8_t sz = bbztable_size(t);

    // Return the size
    bbzvm_pushi(sz);
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbztable_register() {
    bbzvm_function_register(__BBZSTRID_foreach, table_foreach);
    bbzvm_function_register(__BBZSTRID_filter,  table_filter);
    bbzvm_function_register(__BBZSTRID_map,     table_map);
    bbzvm_function_register(__BBZSTRID_reduce,  table_reduce);
    bbzvm_function_register(__BBZSTRID_size,    table_size);
}