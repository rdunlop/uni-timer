#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "modes.h"

extern UniSensor sensor;
extern UniBuzzer buzzer;

//### Mode 1 - Keypad/Sensor Input Test
//
//- If you press a Key, it will Beep for 100ms
//- If you block the Sensor, or un-block the sensor, it will beep for 100ms
bool last_sensor = false;
void mode1_loop() {
  bool sensor_value = sensor.blocked();
  if (last_sensor != sensor_value) {
    Serial.println("blocked");
    buzzer.beep();
    last_sensor = sensor_value;
  }
}
