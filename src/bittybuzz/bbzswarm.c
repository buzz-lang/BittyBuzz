#include "bbzswarm.h"
#include "bbzutil.h"

#ifndef BBZ_DISABLE_SWARMS

#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS

/**
 * @brief Type for the entry of the set of known swarmlists.
 */
typedef struct PACKED swarmlist_entry_t {
    bbzswarmlist_t swarmlist; /**< @brief Robot's swarmlist. */
    bbzlamport_t lamport;     /**< @brief Entry's Lamport clock. */
} swarmlist_entry_t;

#else // !BBZ_DISABLE_SWARMLIST_BROADCASTS

typedef struct PACKED swarmlist_entry_t {
    bbzswarmlist_t swarmlist; /**< @brief Robot's swarmlist. */
} swarmlist_entry_t;

#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS

/**
 * @brief Constructs a swarmlist entry and sets the swarmlist according
 * to the passed swarm ID. Leaves the Lamport clock unassigned
 * (if applicable).
 * @note Sets BBZVM_ERROR_SWARM if swarm ID >= 8.
 * @param[in] swarm The swarm's ID.
 * @return The created entry.
 */
static swarmlist_entry_t swarmlist_entry_fromswarm(bbzswarm_id_t swarm) {
    if (swarm < 8 * sizeof(bbzswarmlist_t)) {
        swarmlist_entry_t ret = {.swarmlist = 1};
        uint8_t i = swarm;
        while(i > 0) {
            // Assembler dump shows that left-shifting is done by 1 bit at
            // a time on AVR processors. Doing the left shift directly
            // increases code size.
            ret.swarmlist <<= 1;
            --i;
        }
        return ret;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_SWARM);
        return (swarmlist_entry_t){};
    }
}

/**
 * Determines if we already have the swarm list of a certain robot, and
 * if so, pushes it on the stack.
 * @param[in] robot The ID of the robot we wand to find the swarmlist of.
 * @return The existing swarmlist entry, or a null structure otherwise.
 */
static swarmlist_entry_t swarmlist_get(bbzrobot_id_t robot);

/**
 * Sets a robot's entry.
 * @param[in] robot The robot whose swarm list to set.
 * @param[in] entry The entry to set inside the 'swarm' structure.
 */
static void swarmlist_set(bbzrobot_id_t robot, swarmlist_entry_t entry);

/**
 * @brief Gets the ID of a subswarm table.
 * @details The table is expected to be at stack top, and will be popped.
 * @return The ID of the subswarm table.
 */
static bbzswarm_id_t get_id() {
    // Push swarm ID (which pops the table)
    bbzvm_pushs(__BBZSTRID_id);
    bbzvm_tget();

    // Fetch and pop swarm ID
    bbzswarm_id_t swarm = bbzheap_obj_at(bbzvm_stack_at(0))->i.value;
    bbzvm_pop();
    return swarm;
}

/**
 * Pushes a table containing all the fields that a subswarm table has.
 * @param[in] swarm The ID of the swarm that this table is for.
 */
static void make_table(bbzswarm_id_t swarm) {
    // Create table
    bbzvm_pusht();

    // Add swarm id
    bbzvm_pushi(swarm);
    bbzheap_idx_t swarm_obj = bbzvm_stack_at(0);
    bbzvm_pop();
    bbztable_add_data(__BBZSTRID_id, swarm_obj);

    // Add closures
    bbztable_add_function(__BBZSTRID_join,   bbzswarm_join);
    bbztable_add_function(__BBZSTRID_leave,  bbzswarm_leave);
    bbztable_add_function(__BBZSTRID_in,     bbzswarm_in);
    bbztable_add_function(__BBZSTRID_select, bbzswarm_select);
    bbztable_add_function(__BBZSTRID_exec,   bbzswarm_exec);
#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
    bbztable_add_function(__BBZSTRID_others, bbzswarm_others);
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS
}

/****************************************/
/****************************************/

/**
 * Constructs the 'swarm' table and its dependencies.
 * @param[in] swarm Position, in the heap, of the swarm table.
 */
static void swarm_construct(bbzheap_idx_t swarm) {
    // Set swarm table
    vm->swarm.hpos = swarm;

    // Create swarmstack
    bbzdarray_new(&vm->swarm.swarmstack);

    // Make stuff permanent
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->swarm.hpos));
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->swarm.swarmstack));

#ifdef BBZ_DISABLE_SWARMLIST_BROADCASTS
    // Initialize swarmlist.
    vm->swarm.my_swarmlist = 0;
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS
}

void bbzswarm_register() {
    bbzvm_pushs(__BBZSTRID_swarm);

    // Create the 'swarm' table
    bbzvm_pusht();

    // Construct the 'swarm' structure.
    bbzheap_idx_t s = bbzvm_stack_at(0);
    swarm_construct(s);

    // Add some fields to the table (most common fields first)
    bbztable_add_function(__BBZSTRID_create,       bbzswarm_create);
    bbztable_add_function(__BBZSTRID_id,           bbzswarm_id);
#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
    bbztable_add_function(__BBZSTRID_intersection, bbzswarm_intersection);
    bbztable_add_function(__BBZSTRID_union,        bbzswarm_union);
    bbztable_add_function(__BBZSTRID_difference,   bbzswarm_difference);

    // Create our own swarm list
    bbzswarm_addmember(vm->robot, 0); // Add us as member of swarm 0, which creates the entry.
    bbzswarm_rmmember(vm->robot, 0); // Immediately remove us from swarm 0. The entry will still exist.
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS

    // Table is stack top, and string 'swarm' is stack #1. Register it.
    bbzvm_gstore();
}

/****************************************/
/****************************************/

/**
 * Base for bbzswarm_addmember() and bbzswarm_rmmember().
 * @param[in] robot The ID of the robot. In non-broadcasting configuration,
 * this value must be equal to the current robot's ID otherwise
 * BBZVM_ERROR_OUTOFRANGE is set.
 * @param[in] swarm The ID of the swarm.
 * @param[in] should_add Whether the robot should be added to or removed from the swarm.
 * @return The swarm list of the robot we added to/removed from the swarm.
 */
static bbzswarmlist_t addrm_member(bbzrobot_id_t robot,
                                   bbzswarm_id_t swarm,
                                   uint8_t should_add) {
    swarmlist_entry_t entry    = swarmlist_entry_fromswarm(swarm);
    swarmlist_entry_t existing = swarmlist_get(robot);
    if (should_add) {
        existing.swarmlist |= entry.swarmlist;
    }
    else {
        existing.swarmlist &= ~entry.swarmlist;
    }
    swarmlist_set(robot, existing);
    return existing.swarmlist;
}

bbzswarmlist_t bbzswarm_addmember(bbzrobot_id_t robot,
                                  bbzswarm_id_t swarm) {
    return addrm_member(robot, swarm, 1);
}

/****************************************/
/****************************************/

bbzswarmlist_t bbzswarm_rmmember(bbzrobot_id_t robot,
                                 bbzswarm_id_t swarm) {
    return addrm_member(robot, swarm, 0);
}

/****************************************/
/****************************************/

void bbzswarm_refresh(bbzrobot_id_t robot,
                      bbzswarmlist_t swarmlist) {
    swarmlist_entry_t entry = {.swarmlist = swarmlist};
    swarmlist_entry_t existing = swarmlist_get(robot);
    existing.swarmlist = entry.swarmlist; // Copy into existing entry to preserve
                                          // the Lamport clock, if applicable.
    swarmlist_set(robot, existing);
}

/****************************************/
/****************************************/

uint8_t bbzswarm_isrobotin(bbzrobot_id_t robot,
                           bbzswarm_id_t swarm) {
    swarmlist_entry_t entry = swarmlist_entry_fromswarm(swarm);
    swarmlist_entry_t existing = swarmlist_get(robot);
    return (existing.swarmlist & entry.swarmlist) != 0;
}

/****************************************/
/****************************************/


// ======================================
// =        BUZZ SWARM CLOSURES         =
// ======================================

void bbzswarm_create() {
    bbzvm_assert_lnum(1);

    uint16_t swarm = bbzheap_obj_at(bbzvm_locals_at(1))->i.value;

    if (swarm < 8) {
        make_table(swarm);
    }
    else {
        bbzvm_pushnil();
        bbzvm_seterror(BBZVM_ERROR_SWARM);
    }

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_id() {
    bbzvm_assert_exec(bbzvm_locals_count() <= 1, BBZVM_ERROR_LNUM);

    if (!bbzdarray_isempty(vm->swarm.swarmstack)) {
        uint16_t stack_depth;

        // Get stack depth (defaults to 0)
        if (bbzvm_locals_count() == 0) {
            stack_depth = 0;
        }
        else {
            stack_depth = bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
        }

        // Make sure we have enough elements
        uint16_t sz = bbzdarray_size(vm->swarm.swarmstack);
        bbzvm_assert_exec(
            stack_depth < sz,
            BBZVM_ERROR_OUTOFRANGE);

        // Get swarm ID.
        bbzheap_idx_t v;
        bbzdarray_get(vm->swarm.swarmstack, sz - stack_depth - 1, &v);
        bbzvm_push(v);
    }
    else {
        // Swarmstack empty. Push nil instead.
        bbzvm_pushnil();
        bbzvm_seterror(BBZVM_ERROR_OUTOFRANGE);
    }

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_join() {
    bbzvm_assert_lnum(0);

    bbzvm_lload(0); // Push table we are calling 'join' on.
    bbzswarm_id_t swarm = get_id();
    bbzswarmlist_t swarmlist = bbzswarm_addmember(vm->robot, swarm);
#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
        // TODO Broadcast our updated swarmlist.
#else // !BBZ_DISABLE_SWARMLIST_BROADCASTS
        RM_UNUSED_WARN(swarmlist);
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzswarm_leave() {
    bbzvm_assert_lnum(0);

    bbzvm_lload(0); // Push table we are calling 'leave' on.
    bbzswarm_id_t swarm = get_id();
    bbzswarmlist_t swarmlist = bbzswarm_rmmember(vm->robot, swarm);
#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
        // TODO Broadcast our updated swarmlist.
#else // !BBZ_DISABLE_SWARMLIST_BROADCASTS
        RM_UNUSED_WARN(swarmlist);
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzswarm_in() {
    bbzvm_assert_lnum(0);

    // Get swarm ID
    bbzvm_lload(0); // Push table we are calling 'in' on.
    bbzswarm_id_t swarm = get_id();

    // Get my swarm list
    uint8_t in = bbzswarm_isrobotin(vm->robot, swarm);
    bbzvm_pushi(in);

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_select() {
    bbzvm_assert_lnum(1);

    uint8_t should_join = bbztype_tobool(bbzheap_obj_at(bbzvm_locals_at(1)));
    if (should_join) {
        bbzvm_lload(0); // Push table we are calling 'select' on.
        bbzswarm_id_t swarm = get_id();
        swarmlist_entry_t entry = swarmlist_entry_fromswarm(swarm);
        swarmlist_entry_t existing = swarmlist_get(vm->robot);
        existing.swarmlist |= entry.swarmlist; // Copy into existing entry to preserve
                                               // the Lamport clock, if applicable.
        swarmlist_set(vm->robot, existing);
    }

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzswarm_exec() {
    bbzvm_assert_lnum(1);
    bbzvm_assert_type(bbzvm_locals_at(1), BBZTYPE_CLOSURE);

    // Get swarm ID and push it on the swarmstack
    bbzvm_lload(0); // Push table we are calling 'exec' on.
    bbzswarm_id_t swarm = get_id();
    if (bbzswarm_isrobotin(vm->robot, swarm)) {
        bbzvm_pushi(swarm);
        bbzheap_idx_t swarm_obj = bbzvm_stack_at(0);
        bbzvm_pop();

        // Push swarmstack
        bbzdarray_push(vm->swarm.swarmstack, swarm_obj);

        // Call closure
        bbzvm_lload(1); // Push closure
        bbzvm_closure_call(0);

        // Pop swarmstack
        bbzdarray_pop(vm->swarm.swarmstack);
    }

    bbzvm_ret0();
}

/****************************************/
/****************************************/

// ------------------------------------- // TODO This implementation is currently not
// -     WITH SWARMLIST BROADCASTS     - // necessary because 'neighbors.[non]kin' are
// ------------------------------------- // not implemented.
#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS

static swarmlist_entry_t swarmlist_get(bbzrobot_id_t robot) {
    bbzvm_push(vm->swarm.hpos);
    bbzvm_pushi(robot);
    bbzvm_tget();
    bbzobj_t* entry = bbzheap_obj_at(bbzvm_stack_at(0));
    bbzvm_pop();

    // Entry already exists?
    if (!bbztype_isnil(*entry)) {
        // Yes ; return it
        return *(swarmlist_entry_t*)&entry->i.value;
    }
    else {
        // No ; return zero entry.
        return (swarmlist_entry_t){};
    }

}

static void swarmlist_set(bbzrobot_id_t robot, swarmlist_entry_t entry) {
    bbzvm_push(vm->swarm.hpos);
    bbzvm_pushi(robot);
    bbzvm_pushi(*(int16_t*)&entry);
    bbzvm_tput();
}

/****************************************/
/****************************************/

void bbzswarm_rmentry(bbzrobot_id_t robot) {
    bbztable_set(vm->swarm.hpos, bbzint_new(robot), vm->nil);
}

/****************************************/
/****************************************/

void bbzswarm_intersection() {
    bbzvm_assert_lnum(3);

    // TODO Implement and unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_union() {
    bbzvm_assert_lnum(3);

    // TODO Implement and unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_difference() {
    bbzvm_assert_lnum(3);

    // TODO Implement and unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_others() {
    bbzvm_assert_lnum(1);

    // TODO Implement and unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

// -------------------------------------
// -   WITHOUT SWARMLIST BROADCASTS    -
// -------------------------------------
#else // !BBZ_DISABLE_SWARMLIST_BROADCASTS

static swarmlist_entry_t swarmlist_get(bbzrobot_id_t robot) {
    if (robot == vm->robot) {
        return (swarmlist_entry_t){.swarmlist = vm->swarm.my_swarmlist};
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_OUTOFRANGE);
        return (swarmlist_entry_t){.swarmlist = ~0};
    }
}

static void swarmlist_set(bbzrobot_id_t robot, swarmlist_entry_t entry) {
    if (robot == vm->robot) {
        vm->swarm.my_swarmlist = entry.swarmlist;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_OUTOFRANGE);
    }
}

#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS
#else // !BBZ_DISABLE_SWARMS

void bbzswarm_dummy()    {                bbzvm_ret0();}
void bbzswarm_dummyret() {bbzvm_pushnil();bbzvm_ret1();}

#endif // !BBZ_DISABLE_SWARMS