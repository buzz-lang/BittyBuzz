/**
 * @file bbzstrids.h
 * @brief Defines the string ids for the BittyBuzz symbols.
 */

#ifndef BBZSTRIDS_H
#define BBZSTRIDS_H

/**
 * @brief String IDs associated with some strings that BittyBuzz needs.
 */
typedef enum {
    __BBZSTRID_ID = 0,
    __BBZSTRID_SWARM,
    __BBZSTRID_CREATE,
    __BBZSTRID_SELECT,
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
    __BBZSTRID_DISTANCE,
    __BBZSTRID_AZIMUTH,
    __BBZSTRID_ELEVATION,
    __BBZSTRID_FOREACH,
    __BBZSTRID_MAP,
    __BBZSTRID_REDUCE,
    __BBZSTRID_FILTER,
    __BBZSTRID_LISTEN,
    __BBZSTRID_IGNORE,
    __BBZSTRID_BROADCAST,
    __BBZSTRID_STIGMERGY,
    __BBZSTRID_PUT,
    __BBZSTRID_GET,
    __BBZSTRID_SIZE,
    __BBZSTRID_ONCONFLICT,
    __BBZSTRID_ONCONFLICTLOST,
    __BBZSTRID_COUNT__ /**< @brief Number of BittyBuzz string IDs. */
} __Bbzstr_id_t;

#endif // !BBZSTRIDS_H