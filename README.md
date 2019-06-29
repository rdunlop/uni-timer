# Design Notes

When the system is powered on, it will start by going through a self-test, and then it will move into mode 1.

## Modes

- Mode 1 - Keypad/Sensor Input Test
- Mode 2 - GPS/Printer/SD Test
- Mode 3 - Sensor Tuning
- Mode 4 - Race Setup
- Mode 5 - Race Run (Start Line)
- Mode 6 - Race Run (Finish Line)

To change modes, press the desired mode number and # sign on the keypad at the same time.

### Mode 0 - Self-Test

- Display 8888
- Wait 3 seconds
- Test reading the paper-tray, SD card, and GPS.
- display 6ood or bAd

### Mode 1 - Keypad/Sensor Input Test

- If you press a Key, it will Beep for 100ms, and display the number on the display.
- If you press A, it will display A
- If you press B, it will display b
- If you press C, it will display C
- If you press D, it will display d
- If you block the Sensor, or un-block the sensor, it will display 5En5 and beep for 100ms

### Mode 2 - GPS/Printer/SD Test

- If you press A, it will show the GPS time, and beep positively.
- If you press B, it will show print a test line on the printer.
- If you press C, it will test writing/reading from the SD card, and display either 6ood or bAd

### Mode 3 - Sensor Tuning

- When the Sensor beam is crossed, no noise. When the sensor is not-crossed, beep continuously.

### Mode 4 - Race Setup

- If you press A, toggle between S/F on digit 1
- If you press B, toggle between B/A/E on digit 2
- If you press C, toggle between U/D on digit 3
- If you press D, toggle between 1..9 on digit 4.

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
