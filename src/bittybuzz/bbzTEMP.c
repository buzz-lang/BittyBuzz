// TODO Remove this file and use a custom kilolib.

#include "bbzTEMP.h"

#ifdef BBZCROSSCOMPILING

#include <avr/io.h>


void set_led(int8_t color) {
    uint8_t rgb = RGB(
        (color & 1) ? 1 : 0,
        (color & 2) ? 1 : 0,
        (color & 4) ? 1 : 0);

    // Got all of this from the kilolib
    if (rgb&(1<<0))
        DDRD |= (1<<5);
    else
        DDRD &= ~(1<<5);

    if (rgb&(1<<1))
        DDRD |= (1<<4);
    else
        DDRD &= ~(1<<4);

    if (rgb&(1<<2))
        DDRC |= (1<<3);
    else
        DDRC &= ~(1<<3);

    if (rgb&(1<<3))
        DDRC |= (1<<2);
    else
        DDRC &= ~(1<<2);

    if (rgb&(1<<4))
        DDRC |= (1<<5);
    else
        DDRC &= ~(1<<5);

    if (rgb&(1<<5))
        DDRC |= (1<<4);
    else
        DDRC &= ~(1<<4);

    _delay_ms(400.0);
}

void bin_count(uint16_t val, uint8_t size) {
    set_led(O);
    set_led(W);
    set_led(O);
    set_led(W);
    for (uint8_t i = 0; i < 8*size; ++i) {
        set_led(O);
        if (val & 1) {
            set_led(G);
            set_led(G);
        }
        else {
            set_led(M);
            set_led(M);
        }
        val >>= 1;
    }
    set_led(W);
    set_led(O);
    set_led(W);
    set_led(O);
}

/*

// -------------------
// - DEBUGGING STUFF -
// -------------------


#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define rx_bitcycles 269
#define rx_msgcycles (11*rx_bitcycles)
#define EEPROM_TXMASK (uint8_t*)0x90
#define TX_MASK_MAX   ((1<<0)|(1<<1)|(1<<2)|(1<<6)|(1<<7))
#define TX_MASK_MIN   ((1<<0))

#define ports_off() {\
    DDRB = 0;\
    DDRC = 0;\
    DDRD = 0;\
    PORTB = 0;\
    PORTC = 0;\
    PORTD = 0;\
}

#define ports_on() {\
    DDRD |= (1<<2);\
    PORTD |= (1<<2);\
}

#define tx_timer_setup() {\
    TCCR0A = 0;\
    TCCR0B = (1<<CS02)|(1<<CS00);\
    OCR0A = 0xFF;                \
    TIMSK0 = (1<<OCIE0A);        \
}

#define rx_timer_setup() {\
    TCCR1A = 0;\
    TCCR1B = 0;             \
    OCR1A = rx_msgcycles;   \
    TIMSK1 = (1<<OCIE1A);   \
}

volatile uint8_t rx_busy;          // flag that signals if message is being received
uint8_t rx_leadingbit;             // flag that signals start bit
uint8_t rx_leadingbyte;            // flag that signals start byte
uint8_t rx_byteindex;              // index to the current byte being decoded
uint8_t rx_bytevalue;              // value of the current byte being decoded
volatile uint8_t tx_mask;
volatile uint16_t kilo_tx_period;
uint16_t tx_clock;                 // number of timer cycles we have waited
uint16_t tx_increment;             // number of timer cycles until next interrupt
volatile uint32_t kilo_ticks;      // internal clock (updated in tx ISR)

void kilo_init() {
    cli();

    ports_off();
    ports_on();
    tx_timer_setup();
    rx_timer_setup();

    rx_busy = 0;
    rx_leadingbit = 1;
    rx_leadingbyte = 1;
    rx_byteindex = 0;
    rx_bytevalue = 0;
#ifndef BOOTLOADER
    tx_mask = eeprom_read_byte(EEPROM_TXMASK);
    if (tx_mask & ~TX_MASK_MAX)
        tx_mask = TX_MASK_MIN;
    tx_clock = 0;
    tx_increment = 255;
    kilo_ticks = 0;
    kilo_tx_period = 3906;
#endif
    sei();
}

void kilo_start() { }

int debug_putchar(char c, FILE *stream) {
    UDR0 = c;
    while(!(UCSR0A & (1<<UDRE0)));
    return 0;
}

void debug_init() {
    static FILE debug_stdout = FDEV_SETUP_STREAM(debug_putchar, NULL, _FDEV_SETUP_WRITE);
    cli();
    DDRD |= (1<<1);                                 // Set UART TxD pin as output
#ifndef BAUD
#define BAUD 38400
#endif
#include <util/setbaud.h>
    UBRR0 = UBRR_VALUE;
#if USE_2X
    UCSR0A |= (1<<U2X0);
#else
    UCSR0A &= ~(1<<U2X0);
#endif
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);              // No parity, 8 bits comm, 1 stop bit
    UCSR0B |= (1<<TXEN0);                           // Enable transmission
    //debug_init_extra();
    stdout = &debug_stdout;
    sei();
}

typedef struct {
    uint8_t data[9]; ///< message payload.
    uint8_t type;    ///< message type.
    uint16_t crc;    ///< message crc.
} message_t;


ISR(TIMER0_COMPA_vect) {
    tx_clock += tx_increment;
    tx_increment = 0xFF;
    OCR0A = tx_increment;
    kilo_ticks++;

    if(!rx_busy && tx_clock>kilo_tx_period) {
        message_t *msg = kilo_message_tx();
        if (msg) {
            if (message_send(msg)) {
                kilo_message_tx_success();
                tx_clock = 0;
            } else {
                tx_increment = rand()&0xFF;
                OCR0A = tx_increment;
            }
        }
    }
}*/

#else // BBZCROSSCOMPILING

#include <stdio.h>

char* led_desc[] = {"OFF", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE"};

void set_led(uint8_t color) {
    printf("Led is now %s.\n", led_desc[color]);
}

#endif // BBZCROSSCOMPILING