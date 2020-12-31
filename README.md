# Robin's UniTimer (version 3)

This codebase is the Arduino code which is installed on the physical Robin's UniTimer v3.
Related projects:
- The PCB Design for the UniTimer is available, if you want to get your own printed. (TBD)
- I printed my PCBs at JLBPCB.
- The Android/iOS App code is also available (TBD)

We use the UniTimer to record the time of various competitors when doing unicycle races.
The device can be configured for many different racing situations.
The main interface is an application running on an Android phone, communicating with the UniTimer over Bluetooth.

The UniTimer system provides the following capabilities:
- GPS-Synchronized timing (very accurate)
- Optical sensor for finish line judging
- SD-Card written results log
- Bluetooth connection to a monitoring/configuration application for the judge
- Buzzer for audio feedback

## How To Use

When the device is powered on, it will begin with a self-test.
If it passes the self-test, it will make a happy beep and then start searching for GPS.
It will then beep once every 5 seconds until it receives GPS lock.
Once it achives GPS lock, it will make a happy beep again, and will stop beeping every 5 seconds.
It will then enter the MODE it was last configured to be in (see below).

If it fails the self-test, it will make 3 fast beeps, and will beep to indicate the error.
- 2 Beep - missing SD card
- 3 Beep - missing GPS sensor

## Connecting to the device via Bluetooth

In order to connect to the device, you need to know the device name.
This is written on the device box.

NOTE: the device name is stored on the SD Card, so if you swap SD Cards, the device name may be changed.
By default, the device name is SET_THIS_NAME, and can be set via the bluetooth App.

## Troubleshooting

- No beeping when turned on: Check power input, check for LED lights on the adapter boards, check for loose adapter boards
- Beeping every 5 seconds. See "How To Use" above
- Sensor not detecting

## Logs

- If the SD card is available, the device logs all events to a logs.txt
- Each log record includes:
  - Timestamp
  - Event type
  - Event Data

## Configuration

- If the SD card is available, the current configuration is read from config.txt
- The configuration format is:
  - Configuration name
  - Configuration value
- Whenever a configuration is changed, the whole config.txt is re-written
- The config.txt is only READ from during initial boot.

### Modes

- Mode 0 - Self-test
TBD
- Mode 1 - Keypad/Sensor Input Test
- Mode 2 - GPS/Printer/SD Test
- Mode 3 - Sensor Tuning
- Mode 4 - Race Setup
- Mode 5 - Race Run (Start Line)
- Mode 6 - Race Run (Finish Line)

Dev Note: The Mode is Read/Write via MODE_UUID.

### Mode 0 - Self-Test

- Beep
- Wait 1 second
- Test that it can write + read to the SD card, Beep result
- Wait 1 second
- Test GPS device is present, Beep result.
- wait 1 second
- Happy Beep, Read config, or move into Mode 1.

### Mode 1 - Sensor Input Test

- If you block the Sensor, or un-block the sensor, it will notify via SENSOR_UUID and beep for 100ms
- You cannot exit this mode unless GPS lock is achieved
- While in this mode, BUZZER_UUID, SENSOR_UUID, CURRENT_TIME_UUID are active.

### Mode 3 - Sensor Tuning

Use this mode when attempting to line up the sensor and reflector across the racing line.

- When the Sensor beam is crossed, no noise. When the sensor is not-crossed, beep continuously.

- While in this mode, the following are active: BUZZER_UUID SENSOR_UUID CURRENT_TIME_UUID

### Mode 4 - ?

### Mode 5 - Race Run (Start Line)

- If you enter a number on the keypad, display that number, and allow up to 3 numbers to be entered.
- If you enter a 4th number, beep and clear the display.
- If you press A, it "Accepts" the number, and makes success music, and continues to show the number on the display.
- Once Accepted, blink the number on the display every second
- If you press D, it clears the number and leaves "Accepted" mode
- When in Accepted state:
  - If the sensor is crossed
    - write the current time to the SD and the printer
    - display 5En5 on the display for 2 seconds and beep for 2 seconds.
- When NOT in Accepted State:
  - If the sensor is crossed
    - display Err and beep
- Press C+* If you need to cancel the previosu rider's start time.
  - This will print and record the cancellation of the previous start time

### Mode 6 - Race Run (Finish Line)

- When a sensor is triggered, display E1 to indicate that you need to enter 1 racer number.
  - It will beep periodically to indicate this
  - If you have 2 times recorded, it will beep twice periodically, etc.
- when you press number keys, display the numbers on the display.
- If you enter more than 3 digits, it will beep and clear
- If you press "A", it will accept the input, and display the time and the racer number to printer/SD
- If you press "C", it will clear the display
- If you press "B", it will duplicate the last time, and create E2
- If you press D+* it will clear the last entry


# Installing this code on the physical device

In order to program the device, you need:
- The Arduino application
- The Supporting libraries
- Connecting cable

## Configuring Arduino Application

Add the necessary board to Arduino software
- Arduino -> Preferences -> Additional Board manager URLs.
  - If you already have something here, add a comma, and then: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
- Tools -> Board -> Board Manager
  - Add the esp32 from Espressif Systems
- Tools -> Board -> Choose "ESP32 Arduino" -> "ESP32 Dev Module"

You may need to install the virtual COM port software (see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/establish-serial-connection.html)

## Adding Arduino Libraries

This project requires the following Arduino libraries:
- Arduino SD 1.2.3 (built-in?)
- Adafruit GPS Library 1.0.3

## Programming with Arduino
- Connect the cable to the Arduino device
- Choose the correct Port
- Program

You may need to reset the arduino if it fails to connect (wait until it fails at least once before trying this)

## Monitoring

The software provides a serial port at 115200 Baud
