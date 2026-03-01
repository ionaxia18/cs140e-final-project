/*
 * Implement the following routines to set GPIO pins to input or 
 * output, and to read (input) and write (output) them.
 *  1. DO NOT USE loads and stores directly: only use GET32 and 
 *    PUT32 to read and write memory.  See <start.S> for thier
 *    definitions.
 *  2. DO USE the minimum number of such calls.
 * (Both of these matter for the next lab.)
 *
 * See <rpi.h> in this directory for the definitions.
 *  - we use <gpio_panic> to try to catch errors.  For lab 2
 *    it only infinite loops since we don't have <printk>
 */
#include "rpi.h"

// void delay_cycles(uint32_t ticks) {
//   while (ticks-- > 0)
//     nop();
// }

void gpio_panic(const char *msg, ...) {
    while(1);
}

// void nop() {}
// See broadcomm documents for magic addresses and magic values.
//
// If you pass addresses as:
//  - pointers use put32/get32.
//  - integers: use PUT32/GET32.
//  semantics are the same.
enum {
    // Max gpio pin number.
    GPIO_MAX_PIN = 53,

    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)

    // <you will need other values from BCM2835!>
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// NOTE: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use ptr calculations versus if-statements!
// void gpio_set_output(unsigned pin) {
//     if(pin > GPIO_MAX_PIN)
//         gpio_panic("illegal pin=%d\n", pin);

//     unsigned sel = pin / 10;
//     unsigned gpio_fsel = GPIO_BASE + 4 * sel;
//     unsigned int value = GET32(gpio_fsel);
//     unsigned shift = 3 * (pin % 10);
//     value &= ~(0x00000007 << shift);
//     value |= 0x00000001 << shift;
//     PUT32(gpio_fsel, value);
// }

// Set GPIO <pin> = on.
void gpio_set_on(unsigned pin) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    // Implement this. 
    // NOTE: 
    //  - If you want to be slick, you can exploit the fact that 
    //    SET0/SET1 are contiguous in memory.
    unsigned gpio_set = gpio_set0 + (pin / 32) * 4;
    unsigned shift = (pin % 32);
    unsigned int value = 0x00000001 << shift;
    PUT32(gpio_set, value);

}

// Set GPIO <pin> = off
void gpio_set_off(unsigned pin) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    // Implement this. 
    // NOTE: 
    //  - If you want to be slick, you can exploit the fact that 
    //    CLR0/CLR1 are contiguous in memory.
    unsigned gpio_clear = gpio_clr0 + (pin / 32) * 4;
    unsigned shift = (pin % 32);
    unsigned int value = 0x00000001 << shift;
    PUT32(gpio_clear, value);

}

// Set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// Return 1 if <pin> is on, 0 if not.
int gpio_read(unsigned pin) {
    unsigned v = 0;

    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    unsigned gpio_level = gpio_lev0 + (pin / 32) * 4;
    unsigned int value = GET32(gpio_level);
    unsigned shift = pin % 32;
    unsigned int mask = 0x00000001 <<  pin;
    value &= mask;
    v = value >> shift;


    return v;
}

void gpio_set_function(unsigned pin, gpio_func_t func) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    if((func & 0b111) != func)
        gpio_panic("illegal func=%x\n", func);

    unsigned sel = pin / 10;
    unsigned gpio_fsel = GPIO_BASE + 4 * sel;
    unsigned int value = GET32(gpio_fsel);
    unsigned shift = 3 * (pin % 10);
    value &= ~(0x00000007 << shift);
    value |= func << shift;
    PUT32(gpio_fsel, value);
}

void gpio_set_input(unsigned pin) {
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

void gpio_set_output(unsigned pin) {
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}
