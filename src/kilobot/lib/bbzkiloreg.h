#ifndef BITTYBUZZ_BBZKILOREG_H
#define BITTYBUZZ_BBZKILOREG_H

extern const uint8_t bcode[];
extern const uint16_t bcode_size;
#include <bbzkilosymgen.h> // A header file generated at compile time containing all the symbole ID

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define bbzkilo_function_register_PASTER(MACRO, func) bbzvm_function_register(MACRO, func);
/**
 * @brief Register a custom function.
 */
#define bbzkilo_function_register(NAME, func) bbzvm_function_register(BBZKILO_SYMID_GEN_ ## NAME, func);

#undef bbzkilo_function_register_PASTER

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BITTYBUZZ_BBZKILOREG_H
