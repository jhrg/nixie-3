/**
 * @brief The pins used by the project, in one place.
 */

// The DS3231 clock chip 1Hz square wave input. Triggers an interrupt
// on both the rising and falling edge for a 2Hz event.
#define CLOCK_1HZ 2

// The push button switch. currently controls brightness. Triggers
// an interrupt handler.
#define INPUT_SWITCH 3  // (PORT D)
// #define INPUT_SWITCH_PORT PORTD3  // i.e., PD3, Used for faster reads

// Choose pins that cannot be used for hardware PWM for all except the
// PWM brightness control.

#define SERIAL_CLK 4  // SERCLK (shift register clock) input. aka SHCP
#define REGISTER_CLK 7  // RDCLK (register clock/latch) input. aka STCP
#define SERIAL_DATA 8   // SER (serial data) input. aka DS

// Other pins on the 74595 shift register chip:
// OE (Output Enable) and SRCLR (Shift Register Clear)
// OE is pulled low all the time for this code.
// SRCLR is pulled high all the time for this code.

// PWM brightness 980Hz on pins 5 and 6, otherwise 480Hz
#define HV_PWM_CONTROL 5

// The flashing colon, flashes once per second. SEPARATOR can be
// any pin between 8 and 13 inclusive (PORT B)
#define SEPARATOR 9
