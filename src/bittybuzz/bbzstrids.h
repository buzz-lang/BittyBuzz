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
typedef enum {
    __BBZSTRID_ID = 0,
    __BBZSTRID_SWARM,
    __BBZSTRID_CREATE,
    __BBZSTRID_SELECT,
    __BBZSTRID_INIT,
    __BBSSTRID_STEP,
    __BBSSTRID_RESET,
    __BBSSTRID_DESTROY,
    __BBZSTRID_JOIN,
    __BBZSTRID_UNSELECT,
    __BBZSTRID_LEAVE,
    __BBZSTRID_IN,
    __BBZSTRID_EXEC,
    __BBZSTRID_INTERSECTION,
    __BBZSTRID_UNION,
    __BBZSTRID_DIFFERENCE,
    __BBZSTRID_OTHERS,
    __BBZSTRID_NEIGHBORS,
    __BBZSTRID_FOREACH,
    __BBZSTRID_MAP,
    __BBZSTRID_REDUCE,
    __BBZSTRID_DISTANCE,
    __BBZSTRID_AZIMUTH,
    __BBZSTRID_ELEVATION,
    __BBZSTRID_COUNT,
    __BBZSTRID_KIN,
    __BBZSTRID_NONKIN,
    __BBZSTRID_DATA,
    __BBZSTRID_FILTER,
    __BBZSTRID_BROADCAST,
    __BBZSTRID_LISTEN,
    __BBZSTRID_IGNORE,
    __BBZSTRID_STIGMERGY,
    __BBZSTRID_PUT,
    __BBZSTRID_GET,
    __BBZSTRID_SIZE,
    __BBZSTRID_COUNT__ /**< @brief Number of BittyBuzz string IDs. */
} __bbzstr_id_t;

#endif // !BBZSTRIDS_H