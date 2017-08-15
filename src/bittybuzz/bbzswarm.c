#include "bbzswarm.h"
#include "bbzutil.h"

#ifndef BBZ_DISABLE_SWARMS

#define SWARM_MASK 0xFF00     /**< @brief Mask for the swarm bitfield of an entry. */
#define AGE_MASK   0x00FF     /**< @brief Mask for the age of an entry. */

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
    bbztable_add_function(__BBZSTRID_others, bbzswarm_others);
}

/**
 * @brief Constructs a 16-bit bitfield where the #SWARM_MASK bits
 * are the swarm bitfield corresponding to the given swarm ID, and the
 * #AGE_MASK bits are set to zero and correspond to the age of the entry.
 * @details <code>swarm_bitfield = 1 << swarm_id</code>.
 * @note Sets BBZVM_ERROR_SWARM if swarm ID >= 8.
 * @param[in] swarm The swarm's ID.
 * @return The 16-bit bitfield to store in the 'swarm' structure.
 */
static uint16_t swarmtoi(bbzswarm_id_t swarm) {
    if (swarm < 8 * sizeof(bbzswarmlist_t)) {
        uint8_t i = swarm;
        uint16_t ret = 0x0100;
        while(i > 0) {
            ret <<= 1; // Assembler dump shows that left-shifting is done by 1 bit at a time on AVR processors.
                       // Doing the left shift directly increases code size.
            --i;
        }
        return ret;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_SWARM);
        return 0;
    }
}


/**
 * Determines if we already have the swarm list of a certain robot, and
 * if so, pushes it on the stack.
 * @param[in] robot The ID of the robot we wand to find the swarmlist of.
 * @return The heap position of the existing swarm list, or nil of none.
 */
static void swarmlist_get(bbzrobot_id_t robot) {
    bbzvm_push(vm->swarm.hpos);
    bbzvm_pushi(robot);
    bbzvm_tget();
}

/**
 * Sets a robot's entry.
 * @param[in] robot The robot whose swarm list to set.
 * @param[in] entry The entry to set inside the 'swarm' structure.
 */
static void swarmlist_set(bbzrobot_id_t robot, uint16_t entry) {
    bbzvm_push(vm->swarm.hpos);
    bbzvm_pushi(robot);
    bbzvm_pushi(entry);
    bbzvm_tput();
}

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

/****************************************/
/****************************************/

/**
 * Constructs the 'swarm' table and its dependencies.
 */
static void swarm_construct(bbzheap_idx_t swarm) {
    // Set swarm table
    vm->swarm.hpos = swarm;

    // Create swarmstack
    bbzdarray_new(&vm->swarm.swarmstack);

    // Make stuff permanent
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->swarm.hpos));
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->swarm.swarmstack));
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
    bbztable_add_function(__BBZSTRID_intersection, bbzswarm_intersection);
    bbztable_add_function(__BBZSTRID_union,        bbzswarm_union);
    bbztable_add_function(__BBZSTRID_difference,   bbzswarm_difference);

    // Create our own swarm list
    bbzswarm_addmember(vm->robot, 0); // Add us as member of swarm 0, which creates the entry.
    bbzswarm_rmmember(vm->robot, 0); // Immediately remove us from swarm 0. The entry will still exist.

    // Table is stack top, and string 'swarm' is stack #1. Register it.
    bbzvm_gstore();
}

/****************************************/
/****************************************/

/**
 * Base for bbzswarm_addmember() and bbzswarm_rmmember().
 * @param[in] robot The ID of the robot.
 * @param[in] swarm The ID of the swarm.
 * @param[in] should_add Whether the robot should be added or removed from the swarm.
 * @return The swarm list of the robot we added to/removed from the swarm.
 */
static bbzswarmlist_t addrm_member(bbzrobot_id_t robot,
                                   bbzswarm_id_t swarm,
                                   uint8_t should_add) {
    uint16_t entry = swarmtoi(swarm);
    swarmlist_get(robot);
    bbzheap_idx_t existing = bbzvm_stack_at(0);
    bbzvm_pop();
    if (!bbztype_isnil(*bbzheap_obj_at(existing))) {
        uint16_t existing_sw_bitfield =
            (bbzheap_obj_at(existing)->i.value) & SWARM_MASK;
        if (should_add) {
            entry = (entry | existing_sw_bitfield);
        }
        else {
            entry = (~entry & existing_sw_bitfield);
        }
    }
    swarmlist_set(robot, entry);

    uint8_t i = 8;
    while (i != 0) {
        entry >>= 1;
        --i;
    }

    bbzswarmlist_t swarmlist = entry;
    return swarmlist;
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
    uint16_t entry = swarmlist;
    uint8_t i = 8;
    while (i != 0) {
        entry <<= 1;
        --i;
    }

    swarmlist_set(robot, entry);
}

/****************************************/
/****************************************/

uint8_t bbzswarm_isrobotin(bbzrobot_id_t robot,
                           bbzswarm_id_t swarm) {
    swarmlist_get(robot);
    bbzheap_idx_t existing = bbzvm_stack_at(0);
    bbzvm_pop();
    if (!bbztype_isnil(*bbzheap_obj_at(existing))) {
        uint16_t entry = bbzheap_obj_at(existing)->i.value;
        uint16_t swarm_bitfield = swarmtoi(swarm);
        return (entry & SWARM_MASK & swarm_bitfield) != 0;
    }
    else {
        return 0;
    }
}

/****************************************/
/****************************************/

void bbzswarm_rmentry(bbzrobot_id_t robot) {
    bbztable_set(vm->swarm.hpos, bbzint_new(robot), vm->nil);
}

// ======================================
// =        BUZZ SWARM CLOSURES         =
// ======================================

void bbzswarm_create() {
    bbzvm_assert_lnum(1);

    uint16_t swarm = bbzheap_obj_at(bbzvm_lsym_at(1))->i.value;

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

void bbzswarm_intersection() {
    bbzvm_assert_lnum(3);

    // TODO Should we calculate the intersection once and for all, or
    //      should we use a recursive definition for the membership?
    // Also unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_union() {
    bbzvm_assert_lnum(3);

    // TODO Should we calculate the union once and for all, or
    //      should we use a recursive definition for the membership?
    // Also unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_difference() {
    bbzvm_assert_lnum(3);

    // TODO Should we calculate the difference once and for all, or
    //      should we use a recursive definition for the membership?
    // Also unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_id() {
    bbzvm_assert_exec(bbzvm_lsym_size() <= 1, BBZVM_ERROR_LNUM);

    if (!bbzdarray_isempty(vm->swarm.swarmstack)) {
        uint16_t stack_depth;

        // Get stack depth (defaults to 0)
        if (bbzvm_lsym_size() == 0) {
            stack_depth = 0;
        }
        else {
            stack_depth = bbzheap_obj_at(bbzvm_lsym_at(1))->i.value;
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
    }

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_others() {
    bbzvm_assert_lnum(1);

    // TODO Should we calculate the complement once and for all, or
    //      should we use a recursive definition for the membership?
    // Also unhide the test for this function.
    bbzvm_seterror(BBZVM_ERROR_NOTIMPL);

    bbzvm_pushnil();
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_join() {
    bbzvm_assert_lnum(0);

    bbzvm_lload(0); // Push table we are calling 'join' on.
    uint8_t swarm = get_id();
    bbzswarmlist_t swarmlist = bbzswarm_addmember(vm->robot, swarm);
    bbzoutmsg_queue_append_swarm(vm->robot, swarmlist, 0); // TODO Lamport clock?

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzswarm_leave() {
    bbzvm_assert_lnum(0);

    bbzvm_lload(0); // Push table we are calling 'leave' on.
    uint8_t swarm = get_id();
    bbzswarmlist_t swarmlist = bbzswarm_rmmember(vm->robot, swarm);
    bbzoutmsg_queue_append_swarm(vm->robot, swarmlist, 0); // TODO Lamport clock?

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
    swarmlist_get(vm->robot);
    uint8_t in = (bbzheap_obj_at(bbzvm_stack_at(0))->i.value & swarmtoi(swarm) & SWARM_MASK) != 0;
    bbzvm_pushi(in);

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzswarm_select() {
    bbzvm_assert_lnum(1);

    if (bbztype_tobool(bbzheap_obj_at(bbzvm_lsym_at(1)))) {
        bbzvm_lload(0); // Push table we are calling 'select' on.
        uint8_t swarm = get_id();
        swarmlist_get(vm->robot);
        uint16_t existing = bbzheap_obj_at(bbzvm_stack_at(0))->i.value;
        uint16_t entry = swarmtoi(swarm) | (existing & SWARM_MASK);
        swarmlist_set(vm->robot, entry);
    }

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzswarm_exec() {
    bbzvm_assert_lnum(1);
    bbzvm_assert_type(bbzvm_lsym_at(1), BBZTYPE_CLOSURE);

    // Get swarm ID and push it on the swarmstack
    bbzvm_lload(0); // Push table we are calling 'exec' on.
    bbzswarm_id_t swarm = get_id();
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

    bbzvm_ret0();
}

#else // !BBZ_DISABLE_SWARMS

void bbzswarm_dummy()    {                bbzvm_ret0();}
void bbzswarm_dummyret() {bbzvm_pushnil();bbzvm_ret1();}

#endif // !BBZ_DISABLE_SWARMS