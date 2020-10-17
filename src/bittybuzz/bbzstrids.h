/**
 * @file bbzstrids.h
 * @brief Definition of string ids for the BittyBuzz symbols.
 */

#ifndef BBZSTRIDS_H
#define BBZSTRIDS_H

/**
 * @brief String IDs associated with some strings that BittyBuzz needs.
 * @details <b>Example</b>: String <code>"stigmergy"</code> is associated with ID
 * <code>__BBZSTRID_STIGMERGY</code>.
 */
typedef enum __bbzstr_id_t {
    __BBZSTRID_id = 0,
    __BBZSTRID_swarm,
    __BBZSTRID_create,
    __BBZSTRID_select,
    __BBZSTRID_init,
    __BBZSTRID_step,
    __BBZSTRID_reset,
    __BBZSTRID_destroy,
    __BBZSTRID_join,
    __BBZSTRID_unselect,
    __BBZSTRID_leave,
    __BBZSTRID_in,
    __BBZSTRID_exec,
    __BBZSTRID_intersection,
    __BBZSTRID_union,
    __BBZSTRID_difference,
    __BBZSTRID_others,
    __BBZSTRID_neighbors,
    __BBZSTRID_foreach,
    __BBZSTRID_map,
    __BBZSTRID_reduce,
    __BBZSTRID_distance,
    __BBZSTRID_azimuth,
    __BBZSTRID_elevation,
    __BBZSTRID_count,
    __BBZSTRID_kin,
    __BBZSTRID_nonkin,
    __BBZSTRID_data,
    __BBZSTRID_filter,
    __BBZSTRID_broadcast,
    __BBZSTRID_listen,
    __BBZSTRID_ignore,
    __BBZSTRID_stigmergy,
    __BBZSTRID_put,
    __BBZSTRID_get,
    __BBZSTRID_size,
    __BBZSTRID_onconflict,
    __BBZSTRID_onconflictlost,
    __BBZSTRID_robot,
    __BBZSTRID_timestamp,
    __BBZSTRID_pos,
    __BBZSTRID_x,
    __BBZSTRID_y,
    __BBZSTRID_orientation,
    __BBZSTRID___INTERNAL_1_DO_NOT_USE__,
    __BBZSTRID___INTERNAL_2_DO_NOT_USE__,
    _BBZSTRID_COUNT_ /**< @brief Number of BittyBuzz string IDs. */
} __bbzstr_id_t;

#endif // !BBZSTRIDS_H