// - GPS
// - This file assumes that the GPS is connected on Serial2

#include "uni_gps.h"
//#define GPSECHO

UniGps::UniGps(int pps_signal_input)
{
  _pps_signal_input = pps_signal_input;
  _last_hour = 0;
}

// Setup function, for initializin Per-second-interrupt singal, and monitoring for GPS data
// over serial2
void UniGps::setup(void (*interrupt_handler)()) {
  newData = false;
  last_gps_print_time = millis();
  
  Serial.println("GPS Initializing");
  pinMode(_pps_signal_input, INPUT);
  attachInterrupt(digitalPinToInterrupt(_pps_signal_input), interrupt_handler, RISING);
  
  Serial2.begin(9600);
  Serial.println("GPS Done init");
}

// NOTE: This is effectively an ISR routine
//
// this method is triggered whenever we have GPS Lock and PPS
// This means that this method is called exactly on the second
// But not necessarily on EVERY second
bool UniGps::synchronizeClocks(unsigned long current_millis) {
  _last_pps_millis = current_millis; // USED OUTSIDE OF THIS ISR
  // time is returned as hhmmsscc
  unsigned long time;
  unsigned long age; // I think that we can do something smart with `age` to deal with loss of lock...maybe?
  gps.get_datetime(NULL, &time, &age);
  byte hour = time / 1000000;

  // Prevent hour from resetting, and causing the result data to be 'negative time'
  if (hour < _last_hour) {
    hour = hour + 24;
  }
  if (hour > _last_hour) {
    _last_hour = hour;
  }

  byte minute = (time / 10000) % 100;
  byte second = (time / 100) % 100;
  _last_gps_time_in_seconds = (hour * 3600) + (minute * 60) + second; // USED OUTSIDE OF THIS ISR
  return true;
}

bool disableInterrupts() {
  uint32_t primask;
  __asm__ volatile("mrs %0, primask\n" : "=r" (primask)::);
  __disable_irq();
  return (primask == 0) ? true : false;
}

void enableInterrupts(bool doit) {
  if (doit) __enable_irq();
}

// return true on success
// return false on error
// return the current hour/minute in GPS time, including milliseconds from
// the PPS pulse
bool UniGps::current_time(TimeResult *output, unsigned long current_millis) {
  unsigned long last_pps_millis;
  unsigned long last_gps_time_in_seconds;

  // this function may be called from an interrupt handler, OR from normal code
  // so we use this mechanism to disable interrupts, if they are enabled, and restore
  // the interrupt state at the end
  bool interruptState = disableInterrupts();
  // disable interrupts while we read volatiles or we might get an
  // inconsistent value (e.g. in the middle of a write to _last_pps_millis)
  last_pps_millis = _last_pps_millis;
  last_gps_time_in_seconds = _last_gps_time_in_seconds;
  enableInterrupts(interruptState);

  unsigned long offset_milliseconds = (current_millis - last_pps_millis);
  output->millisecond = offset_milliseconds % 1000;

  unsigned long current_seconds = last_gps_time_in_seconds + (offset_milliseconds / 1000);

  output->second = current_seconds % 60;
  output->minute = (current_seconds / 60) % 60;
  output->hour = (current_seconds / 3600) % 24;

  return true;
}

void UniGps::readData() {
  while (Serial2.available())
  {
    char c = Serial2.read();
    #ifdef GPSECHO
      Serial.write(c); // uncomment this line if you want to see the GPS data flowing
    #endif
    if (gps.encode(c)) // Did a new valid sentence come in?
      newData = true;
  }
}

bool UniGps::detected() {
  return Serial2.available();
}

void UniGps::printPeriodically() {
  // if millis() or timer wraps around, we'll just reset it
  if (last_gps_print_time > millis())  last_gps_print_time = millis();
  // approximately every 2 seconds or so, print out the current GPS stats
  if (millis() - last_gps_print_time > 2000) { 
    last_gps_print_time = millis(); // reset the timer

    printGPS();

    printGPSDate();
  }
}

// Have we got GPS Lock, and accurate time?
bool UniGps::lock() {
  float flat, flon;
  unsigned long fix_age; // returns +- latitude/longitude in degrees
  gps.f_get_position(&flat, &flon, &fix_age);
  if (fix_age == TinyGPS::GPS_INVALID_AGE) {
    return false;
  } else if (fix_age > 5000) {
    return false;
  } else {
    return true;
  }
}

void UniGps::printGPS() {
  unsigned long chars;
  unsigned short sentences, failed;
  
  Serial.println("---------------------------");
  Serial.println("GPS:");
  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
    newData = false;
  }
  
  gps.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");
}

unsigned long UniGps::charactersReceived() {
  unsigned long chars;
  gps.stats(&chars, NULL, NULL);
  return chars;
}

void UniGps::printGPSDate() {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE) {
    Serial.println("********** ******** ");
  }
  else
  {
    char sz[32];
    snprintf(sz, 32, "%02d/%02d/%02d %02d:%02d:%02d.%03d",
        month, day, year, hour, minute, second, hundredths);
    Serial.println(sz);
  }
}
