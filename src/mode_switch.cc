
#include "mode_switch.h"

#include <Arduino.h>

#include "print.h"

#define SWITCH_INTERVAL 150     // ms
#define SWITCH_PRESS_2S 2000    // 2 Seconds
#define SWITCH_PRESS_5S 5000    // 5 S
#define SWITCH_PRESS_10S 10000  // 10 S

volatile unsigned long input_switch_down_time = 0;
volatile enum switch_press_duration input_switch_press = none;

// Set using an interrupt; see mode_switch.cc/h
volatile int brightness = 0;

const int brightness_count[] = {255, 179, 128, 76, 24, 10};

extern int HV_Control;  // define in main.cpp

#if 0
/**
 * Return true if the input switch was pressed and released.
 *
 * This does not change the state of the input switch state variables.
 */
bool poll_input_button() {
#if DEBUG
    static enum switch_press_duration last_press = none;
    if (last_press != input_switch_press) {
        print("Input switch state: %d\n", input_switch_press);
        last_press = input_switch_press;
    }
#endif
    return input_switch_press != none;
}

// Faster reads than digitalRead(INPUT_SWITCH)
inline bool input_switch_held_down() {
    return PIND & _BV(INPUT_SWITCH_PORT);  // INPUT_SWITCH_PORT --> 3
}
#endif

/**
 * Get the state of the input switch.
 */
enum switch_press_duration get_input_button() {
    return input_switch_press;
}

/**
 * Reset the input button state
 */
void reset_input_button() {
    input_switch_press = none;
}

void input_switch_quick_press() {
    brightness = (brightness == 5) ? 0 : brightness + 1;
    DPRINTV("brightness: %d\n", brightness);
    analogWrite(HV_Control, brightness_count[brightness]);
}

/**
 * Assume that poll_input_switch() returns true.
 *
 * Use the various input switch and input state to perform the correct
 * action. Then reset the input switch state to none.
 */
void process_input_switch_press() {
    switch (get_input_button()) {
        case none:
            return;
        case long_5s:
        case medium_2s:
            break;
        case quick:
            input_switch_quick_press();
            break;
    }
    reset_input_button();
}

// These two IRQ handlers for the input switch share state information about
// the time the switch was pressed, duration of the press and if the switch
// is currently pressed or has been released.
//
// input_switch_press & input_switch_released describe the state of the button
// input_switch_time is the time in ms that the switch was pressed
// input_switch_duration is the time span between press and release
//
void input_switch_push() {
    static unsigned long last_interrupt_time = 0;
    unsigned long now = millis();

    if (now - last_interrupt_time > SWITCH_INTERVAL) {
        digitalWrite(LED_BUILTIN, HIGH);
        input_switch_down_time = now;  // Use this to tell how long it has been held down
        input_switch_press = none;     // This is set when the switch is released
        attachInterrupt(digitalPinToInterrupt(INPUT_SWITCH), input_switch_release, FALLING);
    }

    last_interrupt_time = now;
}

void input_switch_release() {
    static unsigned long last_interrupt_time = 0;
    unsigned long now = millis();

    if (now - last_interrupt_time > SWITCH_INTERVAL) {
        digitalWrite(LED_BUILTIN, LOW);
        unsigned long input_switch_duration = now - input_switch_down_time;
        input_switch_down_time = now;  // TODO Needed?
        attachInterrupt(digitalPinToInterrupt(INPUT_SWITCH), input_switch_push, RISING);

        if (input_switch_duration > SWITCH_PRESS_5S) {
            input_switch_press = long_5s;
        } else if (input_switch_duration > SWITCH_PRESS_2S) {
            input_switch_press = medium_2s;
        } else {
            input_switch_press = quick;
        }

        process_input_switch_press();
    }

    last_interrupt_time = now;
}
