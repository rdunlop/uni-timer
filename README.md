# UniTimer

We use the UniTimer to record the time of various competitors when doing unicycle races.
The device can be configured for many different racing situations.

The UniTimer system provides the following capabilities:
- GPS-Synchronized timing (very accurate)
- Optical sensor for start-line/finish line judging
- SD-Card written results log
- Buzzer for audio feedback

# What are the parts of the UniTimer?

## Front

![Front Face](images/FrontFace.jpg)

This is where you can see the time, racer number, and interact with the UniTimer

## USB cable and PhotoSensor

![External Parts](images/ExternalParts.jpg)

The USB Cable must be plugged in to a power supply (usually a power-brick) to operate.
The Photo Sensor must be pointed at a reflector in order to operate.

## Inside

![Inside](images/Inside.jpg)

The UniTimer is made up of a number of modules which work in concern to operate.

![Inside Annotated](images/InsideAnnotated.jpg)

Inside the box, are:

* A small arduino computer
* A Buzzer
* A power adapter to provide the necessary 10V for the photo sensor
* A GPS device (which provides very accurate time-of-day)
* An SD Card

## How does it work (overview)

When the system is turned on, the GPS seeks a signal, and once it has a singal, it has very accurate (< 2ms in my testing) time-of-day.
There is a keypad, which is used for choosing the mode, and for entering the racer number.

The Arduino uses physical hardware interrupts to do the most-accurate timing of the gps and the photo sensor.

When in "Start mode" the racer number is entered, and then the time that they cross the line is recorded on the SD card. (alternatively, there is a beep-start, which does the prescribed 5 beeps, and records the time that the 5th beep happened, or the time that they crossed)

When in "Finish mode", the system records when the line is crossed, and the judge can enter the racer number after-the-fact (ie: seconds later, when they know the racer's number).

For more details, the code/documentation is found https://github.com/rdunlop/uni-timer/

## Design considerations

Will it work in the mountains? Yes. As long as it can get GPS lock (and ideally, keep GPS lock, to prevent drift), it can work.
It's battery powered, and in my tests, it ran for 36 hours on a small USB-battery-bank.

# How to Use: Instructions

See [This guide](https://docs.google.com/document/d/1nne7qRZOfiDiIRNzK-UGaBbXwPN-JUJtM4CR8g83vfM/edit#heading=h.ewxxmpbs2731) for basic use instructions.

* When the device is powered on, it will begin with a self-test.
* If it passes the self-test, it will make a happy beep and then start searching for GPS.
* It will then beep once every 5 seconds until it receives GPS lock.
* Once it achives GPS lock, it will make a happy beep again, and will stop beeping every 5 seconds.
* It will then enter the MODE it was last configured to be in (see below).

## Logs (TBD)

If the SD card is available, the device logs all events to a logs.txt
- Each log entry includes:
  - Timestamp (%d,%02d,%03d)
  - Event type (human-readable string)
  - Event Data (string)
  - *Note:* The format of the event log is different for different event types

- It will also log to a race-specific log the racer numeber, and time of crossing.

Format (TBD): "%d,%2d,%02d,%03d,-"
- racer number
- hour-of-day
- minutes-of-hour
- seconds (2 digits)
- milliseconds (3 digits)
- fault ("-" or "F")


# Event Hardware Requirements

Before using the UniTimer, we recommend the following additional hardware

* Consider how the sensors will be mounted
  * Will they be stuck into the ground? (is the ground condusive to this)
  * Will they be sandwiched between heavy objects (like rocks)
  * Will they be taped to heavy items (like water bottles)
* Need a USB power brick to provide power
* Need a USB Mini cable (should have an extra cable)
* Should replace CR1220 battery cells for the GPS power system to improve lock aquisition time
* Keep-alive USB Dongle
  * Some USB power packs will go to sleep if they don't detect a high enough load
  * The UniTimer doesn't draw a lot of power, so we may need a USB Keep-alive dongle to fool the power pack
* Tighten the bolts holding the optical sensor to the posts, they come loose over time.
* Box made with laser cutter, plans included. Sourced from https://www.festi.info/boxes.py/UniversalBox?language=en
  * Expected to be cut out of 3mm thick material
  * Display dimensions: https://learn.adafruit.com/assets/108791

# System Start-up

When power is applied to the UniTimer, it will go through the following steps:

* POST (Power-On-Self-Test)
* Beep (check that the buzzer works)
* Display 8888 (check that the diplay works)
* Display Sd (check that the SD is present and readable)
  * display good or bad
* Display gpS (check that the GPS is present)
  * display good or bad
* IF SD is present, and config exists, go into GPS-lock-wait mode if target is mode 5 or 6
* ELSE go into Mode 1

# SD Card Files

The SD card must be formatted with FAT format.
The following files may exist on the SD Card:
- config.txt - the global configuration file, which stores power-lost-persistent configuration
- race_*.txt - various racer files, named differently based on the configuration, storing the results for a race.
- log.txt - the global event log, which stores every significant event.

## File format

The race_*.txt files contain the following format:
%d,,%02d,%02d,%03d,%d

with the fields meaning:
- racer number (1-4 digits)
- DQ, DNF, or empty (if valid)
- minute-of-day (0-2880)
- second-of-minute (00-59)
- millisecond-of-minute (000-999)
- penalties (0-1) - was this an early-start (on a count-down-based start config)

There may also be entries:
- CLEAR_PREVIOUS - indicates that the start judge deemed an incorrect triggering of the sensor, and that the previous result should be discarded.

## Modes

* GPS Lock Wait - Transition mode before moving into Mode 5 or Mode 6

* Mode 0 - Power-On-Self-Test
* Mode 1 - Keypad/Sensor Input Test
* Mode 2 - GPS/SD Test
* Mode 3 - Sensor Tuning
* Mode 4 - Race Setup / Configuration
* Mode 5 - Race Run (Start Line)
* Mode 6 - Race Run (Finish Line)

To change modes, press the desired mode number and # sign on the keypad at the same time.
To see the current mode, press and release the * button.

### Mode 0 - Power-On-Self-Test

* Beep (check that the buzzer works)
* Display 8888 (check that the diplay works)
* Wait 2 seconds
* Display Sd (check that the SD is present and readable)
  * display good or bAd
  * Wait 2 seconsd
* Display gpS (check that the GPS is present)
  * display good or bAd
  * Wait 2 seconds

### Mode 1 - Keypad/Sensor Input Test

- If you press a Key, it will display the number on the display.
- If you press A, it will display A
- If you press B, it will display b
- If you press C, it will display C
- If you press D, it will display d
- If you block the Sensor, or un-block the sensor, it will display 5En5 and beep for 100ms

### Mode 2 - GPS/SD Test

- If you press A, it will show the GPS time (if GPS signal found), otherwise it will wait for lock, and beep positively.
- If you press B, it will display the # chars received from GPS (this number should increment if it can see any GPS signals)
- If you press C, it will test writing/reading from the SD card, and display either 6ood or bAd

### Mode 3 - Sensor Tuning

- When the Sensor beam is crossed, no noise. When the sensor is not-crossed, beep continuously.

### Mode 4 - Race Setup / Configuration

There are 3 sub-modes in this Mode. You can enter each mode by pressing the number on the number pad.
Changes are automatically Saved.

#### Mode 4.1 (Press 1) - Filename (sets the filename on the SD card for the results)

- If you press A, toggle between S/F on digit 1 (indicates Start or Finish)
- If you press B, toggle between B/A/E on digit 2 (indicates beginner, advanced, expert)
- If you press C, toggle between U/D on digit 3 (indicates up or down)
- If you press D, toggle between 1..9 on digit 4.

#### Mode 4.2 (Press 2) - Maximum Racer digits

Defaults to 3 digits for a racer number (e.g. if you enter more than 3 digits, it discards the racer number and you enter it again).

- If you press A, toggle between 3 and 4 digits

#### Mode 4.3 (Press 3) - Start Line Mode

Should the start mode be at-will, or beep-start?

- If you press A, toggle between Start Line Type 1 and Start Line Type 2

Start Line Type 1: once the racer's number is entered, the timer will start when they cross the start line
Start Line Type 2: once the racer's number is entered, the timer will count down with the standard beep-start (beep, beep, beep, beep, beep, BEEP)

#### Mode 4.4 (Press 4) - Finish Line - Racer Spacing

By default, the finish line will not register multiple crossings in rapid succession.
After a crossing, it will ignore more crossings for 500ms (1/2 second)

You can change the duration of delayed spacing here, minimum 0ms, maximum 990ms

- If you press A, Reset to 500ms
- If you press B, Increment by 10ms
- If you press C, Increment by 100ms

If you increment past 9, it will wrap around to 0. (ie: 90ms + 10 ms = 0 ms)

### Mode 5 - Race Run (Start Line)

Before entering this Mode, the system will go through Mode G (GPS Lock mode)

When nothing has been entered, the display will show [-] to indicate sensor and GPS health in the first output character on the display.
The '-' indicates GPS lock, and the moving around the 0 indicates the sensor has not been tripped.

- If you enter a number on the keypad, display that number, and allow up to 3 numbers to be entered (configurable).
- If you enter a 4th number (configurable), beep and clear the display (because you've entered too many digits).
- If you press A, it "Accepts" the number, and makes success music, and continues to show the number on the display.
- Once Accepted, blink the number on the display every second
  - Programmer note: this is written to the event log
- If you press D, it clears the number and leaves "Accepted" mode
  - Programmer note: this is written to the event log
- When in Accepted state:
  - **Instant Start Mode**
    - If the sensor is crossed
      - write the current time to the SD
      - display 5En5 on the display for 2 seconds and beep for 2 seconds.
  - **Countdown Start Mode**
    - Start a Beep countdown: beep, beep, beep, beep, beep, BEEP
    - If the sensor is crossed BEFORE the final beep starts
      - display F on the display for 2 seconds, and beep-beep-beep.
      - Write the scheduled start time to the SD, indicate a fault also occurred
      - Programmer note: Write the crossed-line time to the event-log
- When NOT in Accepted State:
  - If the sensor is crossed
    - display Err and beep
    - Programmer note: this is written to the event log
- Press C+* If you need to cancel the previous rider's start time.
  - This will record the cancellation of the previous start time
  - Programmer note: this is written to the event log

### Mode 6 - Race Run (Finish Line)

Before entering this Mode, the system will go through Mode G (GPS Lock mode)

When nothing has been entered, the display will show [-] to indicate sensor and GPS health in the first output character on the display.
The '-' indicates GPS lock, and the moving around the 0 indicates the sensor has not been tripped.

- When a sensor is triggered, display E1 to indicate that you need to enter 1 racer number.
  - It will beep periodically to indicate this
  - If you have 2 times recorded, it will beep twice periodically, etc.
  - Programmer Note: this is written to the event log
- when you press number keys, display the numbers on the display.
- If you enter more than 3 digits (configurable), it will beep and clear
- If you press "A", it will accept the input, and save the time and the racer number to SD
  - Programmer Note: this is written to the event log
- If you press "C", it will clear the display
- If you press "B", it will duplicate the oldest time, and create E2
  - Programmer Note: this is written to the event log
- If you press D+# it will clear the last entry
  - Programmer Note: this is written to the event log

### GPS Lock Mode

Before allowing a race start/finish line to be run, we need to have a GPS lock so that we have an accurate reference clock.

- Display gps
- display a moving box, waiting for lock
- Once lock is received, display loc, beep successfully, and transition into the desired mode.

# Development Notes

The following notes are helpful for anyone improving this codebase, or doing new revisions of the system.

## v2 PCB Layout

* The easyEDA/schematic.json is an EasyEDA-exported Schematic of the project
* The easyEDA/schematic.pdf is a picture of the Schematic of the project
* The easyEDA/pdf_layout.json is an EasyEDA-exported layout of the project
* The easyEDA/pdf_layout.pdf is a picture of the layout of the project

I printed my boards as jlcpcb.com

## Soldering Notes

The layout includes a DS3231, but it is not currently programmed into the software, and is not populated.
The layout includes 3 buttons, with resistors R8, R9, R10, these are not currently programmed, and are not populated.
The sensor resistors R4 and R1 sometimes need to be adjusted (different values) in order to successfully trip the optocoupler

## Parts list

https://docs.google.com/spreadsheets/d/1kOl7RmKvEk1j6EDeycUz31En5rorrrAdOAqw4xpY5WA/edit?usp=sharing


## Installing this code on the physical device

In order to program the device, you need:
- The Arduino application
- The Supporting libraries
- Connecting cable

### Configuring Arduino Application

- [Arduino IDE](https://www.arduino.cc/en/Main/Software)
- [TeensyDuino Android Plug-in](https://www.pjrc.com/teensy/td_download.html)

Add the necessary board to Arduino software
- Arduino -> Preferences -> Additional Board manager URLs.
  - Add https://adafruit.github.io/arduino-board-index/package_adafruit_index.json to be able to get Adafruit libraries
  - This step MAY be necessary to support the TeensyLC board (unsure)
- Tools -> Board -> Choose "Teensyduino" -> "Teensy LC"

## Adding Arduino Libraries

This project requires the following Arduino libraries:
- Adafruit GPS Library 1.0.3
- AUnit testing framework 1.0.3 (Not currently used?)

## Adafruit Libraries

- [LED Backpack](https://github.com/adafruit/Adafruit_LED_Backpack)
  - [GFX](https://github.com/adafruit/Adafruit-GFX-Library)
- [GPS](https://github.com/adafruit/Adafruit_GPS)
- [Keypad](https://playground.arduino.cc/Code/Keypad/)

## Other Libraries

- [SDFat 1.1.4](https://github.com/greiman/SdFat)  **NOTE: version 1.1.4 recommended at this time**
- [arduino-fsm](https://github.com/jonblack/arduino-fsm)

## Programming with Arduino

- Connect the cable to the Arduino device
- Choose the correct Port
- Choose "Optimize" -> "Smallest Code"
- Program

You may need to press the "Boot" button on the arduino if it fails to connect (wait until it fails at least once before trying this)

## Monitoring

The software provides a serial port at 115200 Baud
