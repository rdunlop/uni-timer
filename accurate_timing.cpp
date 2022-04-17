#include "accurate_timing.h"

unsigned long _pps_start_micros;
unsigned long _interrupt_micros; // this value is cleared once the sensor trip is handled.
unsigned long _last_interrupt_micros; // this value is not cleared after the sensor trip is handled

// CURRENT GPS Date/Time (based on PPS)
TimeResult current_gps_time;
void (*_date_fetch_callback)(byte *, byte *, byte *);

// LAST SENSOR DATE/TIME
TimeResult last_sensor_time;

// Method which we can use in order to get the current year/date/time.
void register_date_callback(void (*date_fetch_callback)(byte *, byte *, byte *)) {
  _date_fetch_callback = date_fetch_callback;
}

// A pulse-per-second (PPS) signal occurs every 1 second,
// And we want to use this to synchronize our clock
// so that when the sensor interrupt is fired,
// we can determine the current time accurately.
// NOTE: The GPS PPS signal will ONLY fire when there is GPS lock.
void pps_interrupt() {
  unsigned long now = micros();
  _pps_start_micros = now;

  byte gps_hour, gps_minute, gps_second;
  _date_fetch_callback(&gps_hour, &gps_minute, &gps_second);
  current_gps_time.hour = gps_hour;
  current_gps_time.minute = gps_minute;
  current_gps_time.second = gps_second;
}

#include "uni_config.h"
extern UniConfig config;

void sensor_interrupt() {
  unsigned long now = micros();
  // Don't trigger 2x in 0.5 seconds (by default 500ms)
  unsigned long required_spacing = config.get_finish_line_spacing() * 1000;
  if (now - _last_interrupt_micros < required_spacing) {
    Serial.println("Ignoring as too close to previous crossing");
    return;
  }
  _interrupt_micros = now;
  _last_interrupt_micros = now;
  last_sensor_time.hour = current_gps_time.hour;
  last_sensor_time.minute = current_gps_time.minute;
  last_sensor_time.second = current_gps_time.second;
  last_sensor_time.millisecond = (_interrupt_micros - _pps_start_micros) / 1000;
}

bool sensor_has_triggered() {
  return _interrupt_micros != 0;
}

unsigned long sensor_interrupt_micros() {
  return _interrupt_micros;
}

void clear_sensor_interrupt_micros() {
  _interrupt_micros = 0;
}

bool lastSensorTime(TimeResult *output) {
  output->hour = last_sensor_time.hour;
  output->minute = last_sensor_time.minute;
  output->second = last_sensor_time.second;
  output->millisecond = last_sensor_time.millisecond;
  return true;
}

bool currentTime(TimeResult *output) {
  output->hour = current_gps_time.hour;
  output->minute = current_gps_time.minute;
  output->second = current_gps_time.second;
  output->millisecond = (micros() - _pps_start_micros) / 1000;
  return true;
}
