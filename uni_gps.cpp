// - GPS
// - This file assumes that the GPS is connected on Serial2

#include "uni_gps.h"

UniGps::UniGps(int pps_signal_input)
{
  _pps_signal_input = pps_signal_input;
}

// Because interrupt handlers run outside of all object-like constructs
// we have a static pointer to a GPS instance, so that we can bounce into
// the interrupt_handler on that object
UniGps * UniGps::instance0_;

// Setup function, for initializin Per-second-interrupt singal, and monitoring for GPS data
// over serial2
void UniGps::setup() {
  pps_start_ms = micros();  
  newData = false;
  last_gps_print_time = millis();
  
  Serial.println("GPS Initializing");
  pinMode(_pps_signal_input, INPUT);
  attachInterrupt(digitalPinToInterrupt(_pps_signal_input), pps_interrupt, RISING);
  instance0_ = this;
  
  Serial2.begin(9600);
}


// NOTE: The GPS PPS signal will ONLY fire when there is GPS lock.
void UniGps::pps_interrupt(){
  instance0_->handle_interrupt();
}

void UniGps::handle_interrupt() {
  unsigned long now = micros();
  Serial.print("GPS PPS: ");
  Serial.println(now - pps_start_ms);
  pps_start_ms = now;
  printGPSDate();
}

void UniGps::loop() {
  while (Serial2.available())
  {
    char c = Serial2.read();
    #ifdef GPSECHO
      Serial.write(c); // uncomment this line if you want to see the GPS data flowing
    #endif
    if (gps.encode(c)) // Did a new valid sentence come in?
      newData = true;
  }
  
  // if millis() or timer wraps around, we'll just reset it
  if (last_gps_print_time > millis())  last_gps_print_time = millis();
  // approximately every 2 seconds or so, print out the current GPS stats
  if (millis() - last_gps_print_time > 2000) { 
    last_gps_print_time = millis(); // reset the timer

    printGPS();

    printGPSDate();
  }
}

void UniGps::printGPS() {
  unsigned long chars;
  unsigned short sentences, failed;
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
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.println(sz);
  }
}