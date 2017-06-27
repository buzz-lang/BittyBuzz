#include "bbzneighbors.h"
#include "bbzutil.h"
#include "bbzoutmsg.h"

/**
 * String ID of the sub-table which contains the neighbors' data tables
 * (the {distance, azimuth, elevation} tables).
 */
#define INTERNAL_STRID_SUB_TBL __BBZSTRID___INTERNAL_1_DO_NOT_USE__

/**
 * String ID of the count subfield.
 */
#define INTERNAL_STRID_COUNT   __BBZSTRID___INTERNAL_2_DO_NOT_USE__

/**
 * @brief Given a neighbor data, pushes a table containing the fields
 * 'distance', 'azimuth' and 'elevation'.
 * @param[in] elem Data of the neighbor structure.
 */
void push_neighbor_data_table(const bbzneighbors_elem_t* elem) {
    bbzvm_pusht();
    // Distance
    bbzvm_pushi(elem->distance);
    bbzheap_idx_t dist = bbzvm_stack_at(0);
    bbzvm_pop();
    bbztable_add_data(__BBZSTRID_distance,  dist);
    // Azimuth
    bbzvm_pushi(elem->azimuth);
    bbzheap_idx_t azim = bbzvm_stack_at(0);
    bbzvm_pop();
    bbztable_add_data(__BBZSTRID_azimuth,   azim);
    // Elevation
    bbzvm_pushi(elem->elevation);
    bbzheap_idx_t elev = bbzvm_stack_at(0);
    bbzvm_pop();
    bbztable_add_data(__BBZSTRID_elevation, elev);
}

/**
 * @brief Performs a foreach. In the case of the xtreme memory implementation,
 * this takes into account whether we are using the 'neighbors' table or a
 * neighbor-like table.
 * @param[in] elem_fun The function to execute on each neighbor.
 * @param[in,out] params Parameters of the function.
 */
void neighborlike_foreach(bbztable_elem_funp elem_fun, void* params);

/**
 * @brief Adds some fields that are common to both the 'neighbors' table and neighbor-like tables gotten from
 * some neighbor operations, such as 'map' or 'filter'.
 * @param[in] count The number of neighbors.
 */
void add_neighborlike_fields(int16_t count) {

#ifndef BBZ_XTREME_MEMORY
    // Add a sub-table which will contain the neighbors' data
    bbzvm_pusht();
    bbzheap_idx_t sub_tbl = bbzvm_stack_at(0);
    bbzvm_pop();
    bbztable_add_data(INTERNAL_STRID_SUB_TBL, sub_tbl);

    // Add neighbor count
    bbzvm_pushi(count);
    bbzheap_idx_t cnt = bbzvm_stack_at(0);
    bbzvm_pop();
    bbztable_add_data(INTERNAL_STRID_COUNT, cnt);
#endif

    // Add function fields
    bbztable_add_function(__BBZSTRID_foreach, bbzneighbors_foreach);
    bbztable_add_function(__BBZSTRID_map,     bbzneighbors_map);
    bbztable_add_function(__BBZSTRID_get,     bbzneighbors_get);
    bbztable_add_function(__BBZSTRID_reduce,  bbzneighbors_reduce);
    bbztable_add_function(__BBZSTRID_count,   bbzneighbors_count);
}

/****************************************/
/****************************************/

/**
 * @brief Constructs the VM's neighbor structure.
 * @param[in] n The neighbor structure.
 */
void neighbors_construct(bbzheap_idx_t n) {
    vm->neighbors.hpos = n;
    bbzneighbors_reset();
}

void bbzneighbors_register() {
    bbzvm_pushs(__BBZSTRID_neighbors);

    // Create the 'neighbors' table
    bbzvm_pusht();

    // Construct the 'neighbors' structure.
    bbzheap_idx_t n = bbzvm_stack_at(0);
    neighbors_construct(n);

    // Add some fields to the table (most common fields first)
    add_neighborlike_fields(0);
    bbztable_add_function(__BBZSTRID_broadcast, bbzneighbors_broadcast);
    bbztable_add_function(__BBZSTRID_listen, bbzneighbors_listen);
    bbztable_add_function(__BBZSTRID_ignore, bbzneighbors_ignore);

    // String 'neighbors' is now stack top, and table is stack #1. Register it.
    bbzvm_gstore();
}

/****************************************/
/****************************************/

void bbzneighbors_reset() {
    // Reset the count
    vm->neighbors.count = 0;

    // Reset 'neighbor''s count subfield
    bbzvm_pushi(0);
    bbzheap_idx_t cnt = bbzvm_stack_at(0);
    bbzvm_pop();
    bbztable_add_data(INTERNAL_STRID_COUNT, cnt);
}

/****************************************/
/****************************************/

void bbzneighbors_broadcast() {
    bbzvm_assert_lnum(2);

    // Get args
    bbzheap_idx_t topic = bbzvm_lsym_at(1);
    bbzheap_idx_t value = bbzvm_lsym_at(2);

    bbzoutmsg_append_broadcast(topic, value);

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzneighbors_listen() {
    bbzvm_assert_lnum(2);

    // Get args
    bbzheap_idx_t topic = bbzvm_lsym_at(1);
    bbzvm_assert_type(topic, BBZTYPE_STRING);
    bbzheap_idx_t c = bbzvm_lsym_at(2);
    bbzvm_assert_exec(bbztype_isclosure(*bbzheap_obj_at(c)), BBZVM_ERROR_TYPE);

    // Set listener
    bbztable_set(vm->neighbors.listeners, topic, c);

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzneighbors_ignore() {
    bbzvm_assert_lnum(1);

    // Get args
    bbzheap_idx_t topic = bbzvm_lsym_at(1);
    bbzvm_assert_type(topic, BBZTYPE_STRING);

    // Remove listener
    bbztable_set(vm->neighbors.listeners, topic, vm->nil);

    bbzvm_ret0();
}

/****************************************/
/****************************************/

/**
 * @brief Function which calls a closure with two areguments.
 * @param[in] key First argument of the closure (the robot ID).
 * @param[in] value Second argument of the closure (the
 * <code>{distance, azimuth, elevation}</code> table).
 * @param[in,out] params The closure to call.
 */
void neighbor_foreach_fun(bbzheap_idx_t key, bbzheap_idx_t value, void *params) {
    bbzheap_idx_t c = *(bbzheap_idx_t*)params;
    
    // Push closure and args
    bbzvm_push(c);
    bbzvm_push(key);
    bbzvm_push(value);

    // Call closure
    bbzvm_closure_call(2);

    // Garbage-collect to reduce memory usage.
    bbzvm_gc();
}

void bbzneighbors_foreach() {
    bbzvm_assert_lnum(1);

    // Get closure
    bbzheap_idx_t c = bbzvm_lsym_at(1);
    bbzvm_assert_type(c, BBZTYPE_CLOSURE);

    // Perform foreach
    neighborlike_foreach(neighbor_foreach_fun, &c);

    bbzvm_ret0();
}

/****************************************/
/****************************************/

/**
 * @brief Function that puts an element in a table.
 * @details The stack is expected to be as follows:
 *
 * 0   -> key
 * 1   -> table
 *
 * @param[in] value The data table (the
 * <code>{distance, azimuth, elevation}</code> table).
 * @param[in] ret The value returned by the user's closure.
 */
typedef void (*put_elem_funp)(bbzheap_idx_t value, bbzheap_idx_t ret);

/**
 * @brief Parameter struct to pass to the element-wise function of 'map'
 * and 'filter'.
 */
typedef struct {
    const bbzheap_idx_t t;  /**< @brief Return table of the map. */
    const bbzheap_idx_t c;  /**< @brief Closure to call. */
    put_elem_funp put_elem; /**< @brief Function that puts a value. */
} neighbor_map_base_t;

/**
 * @brief Element-wise function used by 'map' and 'filter'.
 * @param[in] key Element's key (the robot ID).
 * @param[in] value Element's value (the
 * <code>{distance, azimuth, elevation}</code> table).
 * @param[in,out] params Parameters of the function.
 */
void neighbor_map_base(bbzheap_idx_t key, bbzheap_idx_t value, void* params) {
    neighbor_map_base_t* nm = (neighbor_map_base_t*)params;

    // Save stack size
    uint16_t ss = bbzvm_stack_size();

    // Call closure
    bbzvm_push(nm->c);
    bbzvm_push(key);
    bbzvm_push(value);
    bbzvm_closure_call(2);

    // Make sure we returned a value, and get the value.
    bbzvm_assert_exec(bbzvm_stack_size() > ss, BBZVM_ERROR_RET);
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    bbzvm_pop();

    // Add a value to return table.
    bbzvm_push(nm->t);
    bbzvm_pushi(key);
    nm->put_elem(value, ret);

    // Garbage-collect to reduce memory usage.
    bbzvm_gc();
}

/**
 * @brief Base for 'map' and 'filter'.
 * @param[in] elem_fun Which element-wise function to call.
 */
void neighbors_map_base(put_elem_funp put_elem) {
    bbzvm_assert_lnum(1);

    // Get closure
    bbzheap_idx_t c = bbzvm_lsym_at(1);
    bbzvm_assert_type(c, BBZTYPE_CLOSURE);

    // Make return table
    bbzvm_pusht();
    bbzheap_idx_t ret_tbl = bbzvm_stack_at(0);
    add_neighborlike_fields(0);

    // Perform foreach
    neighbor_map_base_t nm = { .t = ret_tbl, .c = c, .put_elem = put_elem };
    neighborlike_foreach(neighbor_map_base, &nm);

    // Table is already stack top. Return.
    bbzvm_ret1();
}


/**
 * @brief Function that pushes the value retured by the user's closure.
 * @param[in] value The data table (the
 * <code>{distance, azimuth, elevation}</code> table.)
 * @param[in] ret The value returned by the user's closure.
 */
void map_put_elem(bbzheap_idx_t value, bbzheap_idx_t ret) {
    bbzvm_push(ret);
    bbzvm_tput();
}

void bbzneighbors_map() {
    neighbors_map_base(map_put_elem);
}

/****************************************/
/****************************************/

/**
 * @brief Function that puts the data table in the return table if the value
 * retured by the user's closure evaluates to true.
 * @param[in] value The data table (the
 * <code>{distance, azimuth, elevation}</code> table.)
 * @param[in] ret The value returned by the user's closure.
 */
void filter_put_elem(bbzheap_idx_t value, bbzheap_idx_t ret) {
    if (bbztype_tobool(bbzvm_obj_at(ret))) {
        // Add data table to the table
        bbzvm_push(value);
        bbzvm_tput();
    }
    else {
        // Pop key and table
        bbzvm_pop();
        bbzvm_pop();
    }
}

void bbzneighbors_filter() {
    neighbors_map_base(filter_put_elem);
}

/****************************************/
/****************************************/

void neighbor_reduce(bbzheap_idx_t key, bbzheap_idx_t value, void* params) {
    bbzheap_idx_t c = *(bbzheap_idx_t*)params;

    // Get accumulator
    bbzheap_idx_t accum = bbzvm_stack_at(0);
    bbzvm_pop();

    // Save stack size
    uint16_t ss = bbzvm_stack_size();

    // Call closure
    bbzvm_push(c);
    bbzvm_push(key);
    bbzvm_push(value);
    bbzvm_push(accum);
    bbzvm_closure_call(3);

    // Make sure we returned a value.
    bbzvm_assert_exec(bbzvm_stack_size() > ss, BBZVM_ERROR_RET);
    
    // Garbage-collect to reduce memory usage.
    bbzvm_gc();

    // Accumulator is at stack #0.
}

void bbzneighbors_reduce() {
    bbzvm_assert_lnum(2);

    // Get closure
    bbzheap_idx_t c = bbzvm_lsym_at(1);
    bbzvm_assert_type(c, BBZTYPE_CLOSURE);

    // Push accumulator
    bbzvm_lload(2);

    // Perform foreach
    neighborlike_foreach(neighbor_reduce, &c);

    // Accumulator is at stack #0 ; return it.
    bbzvm_ret1();
}

/****************************************/
/****************************************/

// -------------------------------------
// -      REGULAR IMPLEMENTATIONS      -
// -------------------------------------
#ifndef BBZ_XTREME_MEMORY

void bbzneighbors_add(const bbzneighbors_elem_t* data) {
    // Get 'neighbors''s sub-table
    bbzvm_push(vm->neighbors.hpos);
    
    // Increment the neighbor count (we assume it's a new entry).
    ++vm->neighbors.count;
    bbztable_add_data(INTERNAL_STRID_COUNT, vm->neighbors.count);

    // Set data to the sub-table.
    bbzvm_pushs(INTERNAL_STRID_SUB_TBL);
    bbzvm_tget();
    bbzvm_pushi(data->robot);
    push_neighbor_data_table(data);
    bbzvm_tput();
}

/****************************************/
/****************************************/

void bbzneighbors_get() {
    bbzvm_assert_lnum(1);

    // Get args.
    bbzheap_idx_t robot = bbzvm_lsym_at(1);
    bbzvm_assert_type(robot, BBZTYPE_INT);

    // Get the sub-table of the table we are using 'get' on.
    bbzvm_lload(0); // Self table
    bbzvm_pushs(INTERNAL_STRID_SUB_TBL);
    bbzvm_tget();
    bbzheap_idx_t sub_tbl = bbzvm_stack_at(0);

    // Entry found?
    bbzheap_idx_t data;
    if (bbztable_get(sub_tbl, robot, &data)) {
        // Found. Push corresponding data table.
        bbzvm_push(data);
    }
    else {
        // Not found. Push nil instead.
        bbzvm_pushnil();
    }

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_count() {
    bbzvm_assert_lnum(0);

    // Get neighbors' sub-table
    bbzvm_lload(0);
    bbzvm_pushs(INTERNAL_STRID_SUB_TBL);
    bbzvm_tget();

    // Push count and return.
    bbzheap_idx_t neighbor_data = bbzvm_stack_at(0);
    bbzvm_pop();
    bbzvm_pushi(bbztable_size(neighbor_data));
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void neighborlike_foreach(bbztable_elem_funp elem_fun, void* params) {
    // Get the sub-table we are using the algorithm on.
    bbzvm_lload(0); // Self table
    bbzvm_pushs(INTERNAL_STRID_SUB_TBL);
    bbzvm_tget();

    bbzheap_idx_t sub_tbl = bbzvm_stack_at(0);
    bbzvm_pop();

    bbztable_foreach(sub_tbl, elem_fun, params);
}

// -------------------------------------
// - BBZ_XTREME_MEMORY IMPLEMENTATIONS -
// -------------------------------------
#else // !BBZ_XTREME_MEMORY

void bbzneighbors_add(const bbzneighbors_elem_t* data) {
    if (vm->neighbors.count < BBZNEIGHBORS_CAP) {
        // Set data.
        bbzneighbors_elem_t* entry = &vm->neighbors.data[vm->neighbors.count];
        entry->robot     = data->robot;
        entry->distance  = data->distance;
        entry->azimuth   = data->azimuth;
        entry->elevation = data->elevation;
        ++vm->neighbors.count; // Increment neighbor count (we assume it's a new entry).
    }
    else {
        // TODO Issue some kind of warning when we reach max neighbor count?
    }
}

/****************************************/
/****************************************/

/**
 * @brief Parameter struct to pass to the element-wise function of the
 * xtreme 'get' algorithm.
 */
typedef struct {
    const bbzrobot_id_t robot; /**< @brief Sought robot ID. */
    uint8_t found;             /**< @brief Whether we already found the robot ID. */
    bbzheap_idx_t ret;         /**< @brief The found data table. */
} neighbor_get_t;

void neighbor_get(bbzheap_idx_t key, bbzheap_idx_t value, void* params) {
    neighbor_get_t* ng = (neighbor_get_t*)params;
    if (!ng->found && bbzvm_obj_at(key)->i.value == ng->robot) {
        ng->found = 1;
        ng->ret = value;
    }
}

void bbzneighbors_get() {
    bbzvm_assert_lnum(1);

    // Get passed robot ID.
    bbzheap_idx_t robot = bbzvm_lsym_at(1);
    bbzvm_assert_type(robot, BBZTYPE_INT);

    // Perform foreach
    neighbor_get_t ng = {
        .robot = bbzvm_obj_at(robot)->i.value,
        .found = 0,
        .ret = vm->nil };
    neighborlike_foreach(neighbor_get, &ng);

    // Push return value and return
    bbzvm_push(ng.ret);
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzneighbors_count() {
    bbzvm_assert_lnum(0);

    // Get table we are calling 'count' on.
    bbzheap_idx_t self = bbzvm_lsym_at(0);

    // Push neighbor count.
    if (self == vm->neighbors.hpos) {
        //
        // 'neighbors' table ; uses optimized C implementation.
        //
        bbzvm_pushi(vm->neighbors.count);
    }
    else {
        //
        // Neighbor-like table ; uses Buzz implementation.
        //

        // Get 'count' subfield
        bbzvm_lload(0); // Push self table
        bbzvm_pushs(INTERNAL_STRID_COUNT);
        bbzvm_tget();
    }

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void neighborlike_foreach(bbztable_elem_funp elem_fun, void* params) {
    // Get the table we are using the algorithm on.
    bbzheap_idx_t self = bbzvm_lsym_at(0);

    // Perform the right foreach.
    if (self == vm->neighbors.hpos) {
        //
        // 'neighbors' table ; uses optimized C implementation.
        //
        for (uint8_t i = 0; i < vm->neighbors.count; ++i) {
            push_neighbor_data_table(&vm->neighbors.data[i]);
            bbzvm_push(vm->neighbors.data[i].robot);
            bbzheap_idx_t data = bbzvm_stack_at(1);
            bbzheap_idx_t key  = bbzvm_stack_at(0);
            bbzvm_pop();
            bbzvm_pop();
            elem_fun(key, data, params);
            bbzvm_gc(); // Collect the created data table
        }
    }
    else {
        //
        // Neighbor-like table ; uses Buzz implementation.
        //
        bbztable_foreach(self, elem_fun, params);
    }
}

#endif // !BBZ_XTREME_MEMORY
