// - GPS
// - This file assumes that the GPS is connected on Serial2

#include "uni_gps.h"
#include "uni_radio.h" // for troubleshooting
#include "recording.h" // for troubleshooting
extern UniRadio radio;
// #define GPSECHO

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

int chars_skipped = 0;
uint8_t robin_sat_index = 0;
uint8_t robin_msg_id = 0;
void *robin_tracked_ptr = NULL;
#define GPS_BUF_SIZE 300
char gps_buffer[GPS_BUF_SIZE];
int gps_buffer_position = 0;
bool gps_buffer_printed = false;

bool printed_once = false;
void UniGps::readData() {
  if (chars_skipped > 100 && (!printed_once)) {
    printed_once = true;
    Serial.println("ROBIN DEBUG");
    char robinmsg[50];
    snprintf(robinmsg, 49, "tracked_ptr: %p", robin_tracked_ptr);
    log(robinmsg);
    snprintf(robinmsg, 49, "radio valid: %p", radio.statusAddr());
    log(robinmsg);
  }
  radio.checkStatus(41);
  int bytesAvailable;
  char msg[50];
  while ((bytesAvailable = Serial2.available()))
  {
    chars_skipped += 1;
    if (!radio.statusOk() && chars_skipped > 200) {
      snprintf(msg, 29, "ABytes available: %d", bytesAvailable);
      log(msg);
      chars_skipped = 0;
    }
    // radio.checkStatus(42);
    char c = Serial2.read();
    // radio.checkStatus(43);
    gps_buffer[gps_buffer_position] = c;
    if (gps_buffer_position >= GPS_BUF_SIZE) {
      gps_buffer_position = 0;
    }
    #ifdef GPSECHO
      Serial.write(c); // uncomment this line if you want to see the GPS data flowing
    #endif
    if (gps.encode(c)) {
      // Did a new valid sentence come in?
      newData = true;
    }
    if (!radio.statusOk()) {
      if (!gps_buffer_printed) {
        gps_buffer_printed = true;

        char gps_buf_out[GPS_BUF_SIZE + 1 + 10];
        char gps_buf_out_hex[(GPS_BUF_SIZE * 4) + 1];
        for (int i = 0; i < GPS_BUF_SIZE; i++) {
          sprintf(&gps_buf_out[i], "%c", gps_buffer[i] != 0 ? gps_buffer[i] : '-');
          sprintf(&gps_buf_out_hex[i], "0x%d", (uint8_t) gps_buffer[i]);
        }
        snprintf(msg, 49, "GPS BUffer (position: %d)", gps_buffer_position);
        log(msg);
        log(gps_buf_out);
        log(gps_buf_out_hex);
      }
      snprintf(msg, 49, "GPS Info: MsgId:%d SatIndex:%d", robin_msg_id, robin_sat_index);
      log(msg);
    }
    // radio.checkStatus(44);
  }
  radio.checkStatus(54);
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
