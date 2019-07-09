#include "uni_keypad.h"
#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "uni_display.h"
#include "modes.h"

extern UniDisplay display;
extern UniKeypad keypad;
extern UniSensor sensor;
extern UniBuzzer buzzer;

//### Mode 1 - Keypad/Sensor Input Test
//
//- If you press a Key, it will Beep for 100ms, and display the number on the display.
//- If you press A, it will display A
//- If you press B, it will display b
//- If you press C, it will display C
//- If you press D, it will display d
//- If you block the Sensor, or un-block the sensor, it will display 5En5 and beep for 100ms
char last_key = NO_KEY;
bool last_sensor = false;
void mode1_loop() {
  //keypad.printKeypress();

  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key) {
      // New Keypress
      display.show(key);
      Serial.println("Number");
      Serial.println(key);
    }
  }
  last_key = key;

  bool sensor_value = sensor.blocked();
  if (last_sensor != sensor_value) {
    display.sens();
    buzzer.beep();
    last_sensor = sensor_value;
  }
}
