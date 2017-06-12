#include "bbzneighbors.h"
#include "bbzutil.h"

/**
 * @brief Given a neighbor entry, pushes a table containing the fields
 * 'distance', 'azimuth' and 'elevation'.
 * @param[in] entry Entry of the neighbor structure.
 */
void push_neighbor_data_table(const bbzneighbors_elem_t* entry) {
    bbzvm_pusht();
    bbzvm_pushi(entry->distance);
    bbzvm_pushi(entry->azimuth);
    bbzvm_pushi(entry->elevation);
    bbztable_add_data(__BBZSTRID_DISTANCE,  bbzvm_stack_at(2));
    bbztable_add_data(__BBZSTRID_AZIMUTH,   bbzvm_stack_at(1));
    bbztable_add_data(__BBZSTRID_ELEVATION, bbzvm_stack_at(0));
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_pop();
}

/**
 * @brief Adds some fields that are common to all the tables gotten from
 * some neighbor operations, such as 'map', 'kin' or 'filter'.
 */
void add_neighbor_fields() {
    // Add function fields
    bbztable_add_function(__BBZSTRID_FOREACH, bbzneighbors_foreach);
    bbztable_add_function(__BBZSTRID_MAP,     bbzneighbors_map);
    bbztable_add_function(__BBZSTRID_REDUCE,  bbzneighbors_reduce);
    bbztable_add_function(__BBZSTRID_COUNT,   bbzneighbors_count);
    bbztable_add_function(__BBZSTRID_KIN,     bbzneighbors_kin);
    bbztable_add_function(__BBZSTRID_NONKIN,  bbzneighbors_nonkin);
    bbztable_add_function(__BBZSTRID_GET,     bbzneighbors_get);
}

/****************************************/
/****************************************/

void bbzneighbors_construct() {
    vm->neighbors.size = 0;
}

/****************************************/
/****************************************/

void bbzneighbors_register() {
    bbzvm_pushs(__BBZSTRID_NEIGHBORS);

    // Create the 'neighbors' table
    bbzvm_pusht();

    // Add some fields (most common fields first)
    bbztable_add_function(__BBZSTRID_BROADCAST, bbzneighbors_broadcast);
    add_neighbor_fields();
    bbztable_add_function(__BBZSTRID_LISTEN, bbzneighbors_listen);
    bbztable_add_function(__BBZSTRID_IGNORE, bbzneighbors_ignore);

    // String 'neighbors' is now stack top, and table is stack #1. Register it.
    bbzvm_gstore();
}

/****************************************/
/****************************************/

void bbzneighbors_reset() {
    vm->neighbors.size = 0;
}

/****************************************/
/****************************************/

void bbzneighbors_add(uint16_t robot,
                      uint8_t distance,
                      uint8_t azimuth,
                      uint8_t elevation) {

    if (vm->neighbors.size < BBZNEIGHBORS_CAP) {
        vm->neighbors.data[vm->neighbors.size].robot     = robot;
        vm->neighbors.data[vm->neighbors.size].distance  = distance;
        vm->neighbors.data[vm->neighbors.size].azimuth   = azimuth;
        vm->neighbors.data[vm->neighbors.size].elevation = elevation;
        vm->neighbors.size++;
    }
    else {
        // TODO Issue some kind of warning when we reach max neighbor count?
    }
}

/****************************************/
/****************************************/

void bbzneighbors_broadcast() {
    // TODO
    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzneighbors_listen() {
    // TODO
    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzneighbors_ignore() {
    // TODO
    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzneighbors_kin() {
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_nonkin() {
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_get() {
    // Get passed robot ID.
    bbzvm_stack_assert(1);
    bbzheap_idx_t robot_idx = bbzvm_stack_at(0);
    bbzvm_type_assert(robot_idx, BBZTYPE_INT);
    bbzrobot_id_t robot = bbzvm_obj_at(robot_idx)->i.value;

    // Find corresponding entry
    uint8_t found = 0;
    uint8_t i;
    for (i = 0; i < vm->neighbors.size; ++i) {
        if (vm->neighbors.data[i].robot == robot) {
            found = 1;
            break;
        }
    }

    // Entry found?
    if (found) {
        // Found. Push a table with the appropriate values.
        push_neighbor_data_table(&vm->neighbors.data[i]);
    }
    else {
        // Not found. Push nil instead.
        bbzvm_pushnil();
    }
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_foreach() {
    // Get closure
    bbzvm_stack_assert(1);
    bbzheap_idx_t c = bbzvm_stack_at(0);
    bbzvm_type_assert(c, BBZTYPE_CLOSURE);
    bbzvm_pop();

    // Call closure for each neighbor.
    for (uint8_t i = 0; i < vm->neighbors.size; ++i) {
        bbzvm_push(c);
        push_neighbor_data_table(&vm->neighbors.data[i]);
        bbzvm_push(vm->neighbors.data[i].robot);
        bbzvm_pushi(2); // Two arguments
        bbzvm_callc();
    }

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzneighbors_map() {
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_reduce() {
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_filter() {
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_count() {
    bbzvm_ret1();
}
