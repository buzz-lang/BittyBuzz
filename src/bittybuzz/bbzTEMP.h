// TODO Remove this file and use a custom kilolib.

#ifndef BBZTEMP_H
#define BBZTEMP_H

#include <bittybuzz/config.h>

#define O  0            // OFF
#define R  1            // Red
#define G  2            // Green
#define B  4            // Blue
#define C (G + B)       // Cyan
#define M (R + B)       // Magenta
#define Y (R + G)       // Yellow
#define W (R + G + B)   // White

#define RGB(r,g,b) (r&3)|(((g&3)<<2))|((b&3)<<4)

#ifdef BBZ_ROBOT

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



void set_led(int8_t color);
void bin_count(uint16_t val, uint8_t size);

/*
int debug_putchar(char c, FILE *stream);
void debug_init();

void kilo_init();
void kilo_start();
*/

#ifdef __cplusplus
}
#endif // __cplusplus

#else // BBZ_ROBOT

#include <inttypes.h>

void set_led(uint8_t color);

#endif // BBZ_ROBOT

#endif // !BBZTEMP_H