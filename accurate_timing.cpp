#include "accurate_timing.h"

unsigned long _pps_start_micros;
unsigned long _interrupt_micros;

// CURRENT GPS Date/Time (based on PPS)
int gps_year;
byte gps_month, gps_day, gps_hour, gps_minute, gps_second;
void (*_date_fetch_callback)(byte *, byte *, byte *);

// LAST SENSOR DATE/TIME
int year;
byte month, day, hour, minute, second;
int millisecond;

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
  Serial.print("GPS PPS: ");
  Serial.println(now - _pps_start_micros);
  _pps_start_micros = now;

  _date_fetch_callback(&gps_hour, &gps_minute, &gps_second);
}

void sensor_interrupt() {
  unsigned long now = micros();
  // Don't trigger 2x in 0.5 seconds
  if (now - _interrupt_micros < 500000) {
    Serial.println("Ignoring");
    return;
  }
  _interrupt_micros = now;
  Serial.println("INTERRUPTED");
  Serial.println(_interrupt_micros);
  hour = gps_hour;
  minute = gps_minute;
  second = gps_second;
  millisecond = (_interrupt_micros - _pps_start_micros) / 1000;
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

bool currentTime(char *output) {
  sprintf(output, "%02d:%02d:%02d.%03d", hour, minute, second, millisecond);
  return true;
}
