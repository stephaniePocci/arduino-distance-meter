# Arduino Distance Meter

A button-triggered ultrasonic distance meter that displays the latest reading in inches on a 16 x 2 I2C LCD. The sketch uses Arduino_FreeRTOS to coordinate button input, measurement, and display updates.

## Hardware and wiring

Use an Arduino board supported by Arduino_FreeRTOS, an ultrasonic sensor with trigger and echo pins (such as an HC-SR04), a 16 x 2 I2C LCD, and a momentary push button.

| Connection | Arduino pin / setting |
| --- | --- |
| Sensor TRIG | Digital 9 |
| Sensor ECHO | Digital 10 |
| Button signal | Digital 2 |
| LCD SDA / SCL | Your board's SDA / SCL pins |
| LCD I2C address | `0x27` |
| LCD size | 16 columns, 2 rows |

Connect the button between digital pin 2 and the board's logic supply, with an external pull-down resistor (for example, 10 kOhm) from pin 2 to GND. The sketch uses `INPUT`, so an unpressed button must be held LOW by the resistor; a press reads HIGH.

Connect all grounds together and power the sensor and LCD according to their specifications. Match signal voltages to the board; a 5 V echo output requires level shifting when used with a 3.3 V input. On an Uno R3, SDA is A4 and SCL is A5.

## Setup and upload

1. Install the Arduino IDE and board support for your Arduino.
2. Install `Arduino_FreeRTOS` and a compatible `LiquidCrystal_I2C` library through the Library Manager. The LCD library must provide `init()`, `backlight()`, and the usual LCD print methods; libraries with this name can have different APIs. `semphr.h` and `queue.h` come with FreeRTOS.
3. Open `arduino_measure_display/arduino_measure_display.ino`. Keep the sketch in its matching `arduino_measure_display` folder.
4. Check the pin assignments and LCD address near the top of the sketch against your wiring.
5. Select the correct board and port, then verify and upload the sketch.
6. Aim the sensor at a surface and press the button. The LCD should show a value such as `12.3 in`. Release the button before taking another reading.

## How it works

- `TaskButton` polls the button, signals a binary semaphore on a press, and waits for a stable release before accepting another press. It has priority 2; the other tasks have priority 1.
- `TaskMeasure` waits on that semaphore, sends a 10 microsecond trigger pulse, and waits up to 30 milliseconds for an echo. It calculates inches using `(duration / 2.0f) / 74.0f`, where duration is in microseconds.
- A length-one queue holds the latest successful measurement. `xQueueOverwrite()` replaces an unread older value.
- `TaskDisplay` waits for a queued measurement, then prints it to one decimal place with the `in` unit.

The LCD stays blank until the first successful measurement. If no echo arrives before the timeout, no new value is sent and the previous display remains. Holding the button does not repeatedly measure. The binary semaphore does not retain a count of rapid presses.

If queue, semaphore, or task creation fails, setup displays an error and stops. The empty `loop()` leaves execution to the FreeRTOS tasks.

## Hardware check

After upload, check a known distance with a ruler, confirm that holding the button produces only one reading, and confirm that a new press after release updates the display. Test a missing echo as well: the previous reading should remain. The conversion is approximate, so validate accuracy for your sensor and environment.

This repository contains the supplied sketch without functional changes. It has not been compiled or tested on hardware in this environment.
