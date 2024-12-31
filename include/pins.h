/**
 * @brief The pins used by the project, in one place.
 */

// The DS3231 clock chip 1Hz square wave input. Triggers an interrupt
// on both the rising and falling edge for a 2Hz event.
#define CLOCK_1HZ 2

// The push button switch. currently controls brightness. Triggers
// an interrupt handler.
#define INPUT_SWITCH 3  // D3

#define SERIAL_CLK 4  // SHCP aka SRCLK (shift register clock) input

#define HV_PWM_CONTROL 5  // PWM brightness 980Hz on pins 5 and 6, otherwise 480Hz

#define REGISTER_CLK 7  // STCP aka RCLK (register clock/latch) input

#define SERIAL_DATA 8  // DS aka SER (serial data) input

// SEPARATOR can be any pin between 8 and 13 inclusive (PORT B)
#define SEPARATOR 9

#if 0
// Choose pins that cannot be used for hardware PWM
int serialDataPin = 8; // DS aka SER (serial data) input
int clockPin = 4;      // SHCP aka SRCLK (shift register clock) input
int latchPin = 7;      // STCP aka RCLK (register clock/latch) input

// OE (Output Enable) and SRCLR (Shift Register Clear)
// OE is pulled low all the time for this code.
// SRCLR is pulled high all the time for this code.

int HV_Control = 5; 
#endif