#include "accurate_timing.h"

unsigned long _interrupt_millis; // this value is cleared once the sensor trip is handled.
unsigned long _last_interrupt_millis; // this value is not cleared after the sensor trip is handled

// LAST SENSOR DATE/TIME
TimeResult last_sensor_time;

// Method which we can use in order to get the current year/date/time.

#include "uni_gps.h"
extern UniGps gps;

// A pulse-per-second (PPS) signal occurs every 1 second,
// And we want to use this to synchronize our clock
// so that when the sensor interrupt is fired,
// we can determine the current time accurately.
// NOTE: The GPS PPS signal will ONLY fire when there is GPS lock.
void pps_interrupt() {
  unsigned long now = millis();

  gps.synchronizeClocks(now);
}

#include "uni_config.h"
extern UniConfig config;

void sensor_interrupt() {
  unsigned long now = millis();
  // Don't trigger 2x in 0.5 seconds (by default 500ms)
  unsigned long required_spacing = config.get_finish_line_spacing() * 1000;
  if (now - _last_interrupt_millis < required_spacing) {
    Serial.println("Ignoring as too close to previous crossing");
    return;
  }
  _interrupt_millis = now;
  _last_interrupt_millis = now;
  gps.current_time(&last_sensor_time, now);
}

bool sensor_has_triggered() {
  return _interrupt_millis != 0;
}

unsigned long sensor_interrupt_millis() {
  return _interrupt_millis;
}

void clear_sensor_interrupt_millis() {
  _interrupt_millis = 0;
}

bool lastSensorTime(TimeResult *output) {
  output->hour = last_sensor_time.hour;
  output->minute = last_sensor_time.minute;
  output->second = last_sensor_time.second;
  output->millisecond = last_sensor_time.millisecond;
  return true;
}

bool currentTime(TimeResult *output) {
  gps.current_time(output, millis());
  return true;
}
