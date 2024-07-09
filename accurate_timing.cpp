#include "accurate_timing.h"

volatile unsigned long _interrupt_millis; // this value is cleared once the sensor trip is handled.
volatile unsigned long _last_interrupt_millis; // this value is not cleared after the sensor trip is handled

// LAST SENSOR DATE/TIME
volatile TimeResult last_sensor_time;

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
#include "uni_sd.h"
extern UniSd sd;
#include "recording.h" // for print_data_to_log

/* RULES FOR INTERRUPTS
 * Don't attempt to delay, eg: delay (100);
 * You can get the time from a call to millis, however it won't increment, so don't attempt to delay by waiting for it to increase.
 * Don't do serial prints (eg. Serial.println ("ISR entered"); )
 * Don't try to do serial reading.
*/
void sensor_interrupt() {
  unsigned long now = millis();
  // Don't trigger 2x in 0.5 seconds (by default 500ms)
  unsigned long required_spacing = config.get_finish_line_spacing();
  if (now - _last_interrupt_millis < required_spacing) {
    // Serial.println("Ignoring as too close to previous crossing");
  } else {
    _interrupt_millis = now;
    _last_interrupt_millis = now;
    TimeResult temp;
    gps.current_time(&temp, now);
    last_sensor_time.hour = temp.hour;
    last_sensor_time.minute = temp.minute;
    last_sensor_time.second = temp.second;
    last_sensor_time.millisecond = temp.millisecond;
    // print_data_to_log(last_sensor_time); /* ******************** THIS COULD BE A PROBLEM **************** */
  }
}

/* These functions also interact with same variables as the sensor_interrupt ISR, and thus need to be protected */
bool sensor_has_triggered() {
  bool result;
  noInterrupts();
    result = _interrupt_millis != 0;
  interrupts();
  return result;
}

void clear_sensor_interrupt_millis() {
  noInterrupts();
  _interrupt_millis = 0;
  interrupts();
}

bool lastSensorTime(TimeResult *output) {
  noInterrupts();
  output->hour = last_sensor_time.hour;
  output->minute = last_sensor_time.minute;
  output->second = last_sensor_time.second;
  output->millisecond = last_sensor_time.millisecond;
  interrupts();
  return true;
}

bool currentTime(TimeResult *output) {
  gps.current_time(output, millis());
  return true;
}
