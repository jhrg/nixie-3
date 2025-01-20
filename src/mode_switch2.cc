//
//
//  This example and code is in the public domain and may  be used without restriction and
//  without warranty.
//
// Exmple sketch  - Button Switch Using An External Interrupt
// '''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//  This sketch demonstrates the use of a simple button switch which is processed by
//  an external interrupt process.  It presents a very different and alternative approach
//  for associating a button switch to an interrupt.
//
// The sketch is designed  such that button status is only flagged as 'switched' AFTER
// 1. button is pressed  AND then released, AND
// 2. elapse of the debounce period AFTER release
//
//  Note that the associated button interrupt handler function and the button_read()
//  function work together - the interrupt handler starts the on/off process and the
//  button_read() function completes/concludes it.  The interrupt handler can only restart  AFTER
// button reading and debounce is complete.  This ensures that only one  interrupt trigger is
// processed at a time.
//
// The button switch is  wired in a standard configuration with a 10K ohm pull down resister which
//  ensures the digital interrupt pin is kept LOW until the button switch is pressed  and
// raises it to HIGH (+5v).
//
// Operation of the button is demonstrated  by toggling the in built LED on and off.
//
// Based on a tutorial by ronbentley1 at
// From https://projecthub.arduino.cc/ronbentley1/button-switch-using-an-external-interrupt-16d57f

#include <Arduino.h>

#include "mode_switch.h"
#include "pins.h"
#include "print.h"

#define LED LED_BUILTIN  // digital pin connected to LED, for testing of switch code only
bool led_status = LOW;   // start with LED off, for testing of switch code  only

// int button_switch = 2;  // external interrupt  pin

#define switched true                  // value if the button  switch has been pressed
#define triggered true                 // controls  interrupt handler
#define interrupt_trigger_type RISING  // interrupt  triggered on a RISING input
#define debounce 100                    // time to wait in milli secs

#define SWITCH_PRESS_2S 2000    // 2 Seconds
#define SWITCH_PRESS_5S 5000    // 5 S
#define SWITCH_PRESS_10S 10000  // 10 S

volatile enum display_mode the_display_mode = mm_ss;    // FIXME: not changed yet

volatile bool interrupt_process_status = {
    !triggered  // start with no switch press pending,  i.e., false (!triggered)
};

bool initialisation_complete = false;  // inhibit any interrupts until initialisation is complete

//
// ISR for  handling interrupt triggers arising from associated button switch
//
void button_interrupt_handler() {
    if (initialisation_complete == true) {  //  all variables are initialised so we are okay to continue to process this interrupt
        if (interrupt_process_status == !triggered) {
            // new interrupt so okay  start a new button read process -
            // now need to wait for button release  plus debounce period to elapse
            // this will be done in the button_read  function
            if (digitalRead(INPUT_SWITCH) == HIGH) {
                // button  pressed, so we can start the read on/off + debounce cycle which will
                //  be completed by the button_read() function.
                digitalWrite(LED, HIGH);
                interrupt_process_status = triggered;  // keep this ISR 'quiet' until button read fully completed
            }
        }
    }
}  // end of button_interrupt_handler

enum switch_press_duration read_button() {
    int button_reading;
    // static variables because we need to retain old values  between function calls
    static bool switching_pending = false;
    static long elapse_timer;
    static long initial_time = 0;
    if (interrupt_process_status == triggered) {
        //  interrupt has been raised on this button so now need to complete
        // the button  read process, ie wait until it has been released
        // and debounce time elapsed
        button_reading = digitalRead(INPUT_SWITCH);
        if (button_reading == HIGH) {
            // switch is pressed, so start/restart wait for button relealse, plus  end of debounce process
            switching_pending = true;
            elapse_timer = millis();  // start elapse timing for debounce checking
            if (initial_time == 0)
                initial_time = elapse_timer;
        }
        if (switching_pending && button_reading == LOW) {
            // switch was pressed, now released, so check  if debounce time elapsed
            if (millis() - elapse_timer > debounce) {
                // debounce time elapsed, so switch press cycle complete
                digitalWrite(LED, LOW);
                switching_pending = false;              // reset for next button press interrupt cycle
                interrupt_process_status = !triggered;  // reopen ISR for business now button on/off/debounce cycle complete
                long elapsed = millis() - initial_time; // measure time from the initial button press
                initial_time = 0;
                DPRINTV("elapsed: %ld\n", elapsed);
                if (elapsed > SWITCH_PRESS_5S)
                    return long_5s;
                else if (elapsed > SWITCH_PRESS_2S)
                    return medium_2s;
                else
                    return quick;
            }
        }
    }
    return none;  // either no press request or debounce  period not elapsed
}  // end of read_button function

void mode_switch_setup() {
#if 0
    pinMode(LED, OUTPUT);
#endif
    pinMode(INPUT_SWITCH, INPUT);
    attachInterrupt(digitalPinToInterrupt(INPUT_SWITCH),
                    button_interrupt_handler,
                    interrupt_trigger_type);
    initialisation_complete = true;  // open interrupt processing for business
}  // end of setup function

#if 0
void loop() {
    // test buton switch and process  if pressed
    if (read_button() == switched) {
        // button on/off cycle now  complete, so flip LED between HIGH and LOW
        led_status = HIGH - led_status;  // toggle state
        digitalWrite(LED, led_status);
    } else {
        // do  other things....
    }
}
#endif