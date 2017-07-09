/**
 * @file bbzstring.h
 * @brief Definition of utility macros for string IDs.
 */

#ifndef BBZSTRING_H
#define BBZSTRING_H

extern const uint8_t bcode[];
extern const uint16_t bcode_size;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Determines the name of the string ID macro corresponding to the
 * given string.
 * @warning All characters that would otherwise make an invalid identifier
 * should be replaced by an underscore, e.g.
 * "2 Swarms" -> BBZSTRING_ID(__Swarms)
 */
#define BBZSTRING_ID(name) BBZSTRID_ ## name

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZSTRING_H
