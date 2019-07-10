// - GPS
// - This file assumes that the GPS is connected on Serial2

#include "uni_gps.h"
//#define GPSECHO

UniGps::UniGps(int pps_signal_input)
{
  _pps_signal_input = pps_signal_input;
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

// return true on success
// return false on error
// return the current hour/minute in GPS time, including milliseconds from
// the PPS pulse
bool UniGps::current_time(byte *hour, byte *minute, byte *second) {
  int year;
  byte month, day, new_hour, new_minute, new_second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &new_hour, &new_minute, &new_second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE) {
    return false;
  }
  *hour = new_hour;
  *minute = new_minute;
  *second = new_second;
  
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

int UniGps::getHourMinuteSecond(int *hour, int *minute, int *second) {
  int year;
  byte month, day, new_hour, new_minute, new_second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &new_hour, &new_minute, &new_second, &hundredths, &age);
  *hour = new_hour;
  *minute = new_minute;
  *second = new_second;
  
  return 1;
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
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d.%03d",
        month, day, year, hour, minute, second, hundredths);
    Serial.println(sz);
  }
}
