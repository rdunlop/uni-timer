/* ****************************************************************************************** */
// UniTimer
//
// This application interacts with an optical sensor device, and prints the results
// to a thermal printer as well as displays to a 7-segment display
//
// Expected Hardware Components
// - SENSOR - Sensor
// - GPS - GPS Sensor, for setting accurate time signal
// - BUZZER - Piezo buzzer
// - BUTTON - Input button
// - SD - MicroSD Storage card
//
// Needed Libraries
// - Download and provide https://github.com/adafruit/SD in the SD folder. (this replaces the SD library included by the GPS library
//
// NOTES:
// [1] The GPS is used to know the absolute time.
//     Based on https://wyolum.com/syncing-arduino-with-gps-time/.
//     Whenever we have GPS lock, we keep track of the offset from micros() for the GPS time
//     and we use that offset whenever we are printing the time.
// [2] Modified the Keypad_I2C library so that it provides the hardware address of the wire bus.
//     I had to change the Keypad_I2C.h file:
//         Keypad_I2C(char* userKeymap, byte* row, byte* col, byte numRows, byte numCols, byte address, byte width = 1) :
//         #if defined(__arm__) && defined(TEENSYDUINO)
//         Keypad(userKeymap, row, col, numRows, numCols), TwoWire(address, i2c0_hardware)
//         #else
//         Keypad(userKeymap, row, col, numRows, numCols)
//         #endif
//         { i2caddr = address; i2cwidth = width;}
//     There may be a way to specify a default-constructor for TwoWire, so that I don't have to do this?
/* ****************************************************************************************** */

// https://www.bastelgarage.ch/index.php?route=extension/d_blog_module/post&post_id=8
// https://github.com/joeyoung/arduino_keypads/blob/master/Keypad_I2C/Keypad_I2C.h

/* ************************* Capabilities flags ******************************************* */
/* Set these flags to enable certain combinations of components */
// #define ENABLE_BLE
#define ENABLE_GPS
// #define ENABLE_SD
#define ENABLE_SENSOR
#define ENABLE_BUZZER
#define ENABLE_ACCURATE_TIMING
// #define DEBUG_GPS

/* *********************** Includes *********************************** */

#ifdef ENABLE_GPS
#include "uni_gps.h"
#endif
#ifdef ENABLE_SD
#include "uni_sd.h"
#endif
#ifdef ENABLE_BUZZER
#include "uni_buzzer.h"
#endif
#ifdef ENABLE_SENSOR
#include "uni_sensor.h"
#endif
#ifdef ENABLE_BLE
#include "uni_ble.h"
#endif

#include "accurate_timing.h"
#include "event_queue.h"
#include "modes.h"
#include "recording.h"
#include "subscribers.h"
#include "uni_config.h"

/* *************************** (Defining Global Variables) ************************** */
// - SENSOR
#define SENSOR_DIGITAL_INPUT 33
// - GPS
#define GPS_PPS_DIGITAL_INPUT 15
#define GPS_DIGITAL_OUTPUT 16 // hardware serial #2
#define GPS_DIGITAL_INPUT 17 // hardware serial #2
// - SD Card
#define SD_SPI_CHIP_SELECT_OUTPUT 5
#define SD_SPI_MOSI_INPUT 23 // The SD library defaults to this set of pins on this board
#define SD_SPI_MISO_INPUT 10 // The SD library defaults to this set of pins on this board
#define SD_SPI_CLK_OUTPUT 9 // The SD library defaults to this set of pins on this board
// - BUZZER
#define BUZZER_DIGITAL_OUTPUT 25

#define LED_BUILTIN 2

/* ************************** Initialization ******************* */

#ifdef ENABLE_SD
UniSd sd(SD_SPI_CHIP_SELECT_OUTPUT);
#endif

#ifdef ENABLE_GPS
UniGps gps(GPS_PPS_DIGITAL_INPUT);
#endif

#ifdef ENABLE_BUZZER
UniBuzzer buzzer(BUZZER_DIGITAL_OUTPUT);
#endif

#ifdef ENABLE_SENSOR
UniSensor sensor(SENSOR_DIGITAL_INPUT);
#endif

#ifdef ENABLE_BLE
UniBle ble;
#endif

// GLOBAL STATE MANAGEMENT
UniConfig config; // No arguments for constructor, hence, no parentheses


// NEW HEADER FILE
void date_callback(byte *hour, byte *minute, byte *second);

/******** ***********************************(set up)*** *************** **********************/
void main_setup () {
  // Common
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  // SENSOR
#ifdef ENABLE_SENSOR
#ifdef ENABLE_ACCURATE_TIMING
  sensor.setupInterruptHandler(&sensor_interrupt);
#endif
#endif

  delay(2000); // wait for serial to connect before starting
  Serial.println("Starting");

  // GPS
#ifdef ENABLE_GPS
  gps.setup(&pps_interrupt);
#endif

#ifdef ENABLE_BUZZER
  buzzer.setup();
#endif

#ifdef ENABLE_SD
  sd.setup();
#endif
buzzer.beep();

  if (config.loadedFromDefault()) {
    Serial.println("Config File not found, loaded defaults");
  } else {
    Serial.println("Config Read Success");
  }
  // the date_callback will be called on each PPS (1/second).
  register_date_callback(date_callback);
#ifdef ENABLE_SENSOR
#ifdef ENABLE_ACCURATE_TIMING
  sensor.attach_interrupt();
#endif
#endif
}

void date_callback(byte *hour, byte *minute, byte *second) {
  if (gps.current_time(hour, minute, second)) {
    Serial.println("PPS Tick");
    char value_string[EVT_MAX_STR_LEN];
    snprintf(value_string, EVT_MAX_STR_LEN, "%02d:%02d:%02d", *hour, *minute, *second);
    push_event(EVT_TIME_CHANGE, value_string);
  }
}

// Check systems, and display Good or Bad on the display
void mode0_run() {
  bool success = true;

#ifdef ENABLE_SD
  if (sd.status()) {
    Serial.println("SD Card OK");
  } else {
    Serial.println("SD Card Error");
    success = false;
  }
#endif

  // TODO: Check GPS


  // Wait 2 seconds
  delay(2000);

  delay(1000);
}

// listen for mode change, and change our internal mode
void mode_callback(uint8_t event_type, char *event_data) {
  if (event_type == EVT_MODE_CHANGE) {
    int previousMode = config.mode();
    if (previousMode == 4) {
      mode4_teardown();
    } else if (previousMode == 5) {
      mode5_teardown();
    } else if (previousMode == 6) {
      mode6_teardown();
    }

    config.setMode(atoi(event_data));
    Serial.println("changing Mode to: ");
    Serial.println(config.mode());

    if (config.mode() == 4) {
      mode4_setup();
    } else if (config.mode() == 5) {
      mode5_setup();
    } else if(config.mode() == 6) {
      mode6_setup();
    }

  }
}
#ifdef ENABLE_BLE
// pass-through method to pass the callback to the
// one-and-only ble object
void bt_notify_callback(uint8_t event_type, char *event_data) {
  ble.bt_notify_callback(event_type, event_data);
}
#endif

// This method is run once-and-only-once when the device
// is first powered on.
// It does Self-test, and then configures the device in the
// appropriate mode (if possible)
// After this method finishes, the loop() method runs over and over again.
void setup() {
  Serial.begin(115200);
  main_setup();

#ifdef ENABLE_BLE
  ble.setup();
  register_subscriber(&bt_notify_callback);
#endif

  register_subscriber(&mode_callback);
  register_subscriber(&publish_time_recorded);
}

// once the main() method completes,
// this method will run over and over again, as quickly
// and frequently as possible.
void loop() {
  gps.loop();
  #ifdef DEBUG_GPS
  gps.printPeriodically();
  #endif
  #ifdef ENABLE_BUZZER
  buzzer.loop();
  #endif
  #ifdef ENABLE_SENSOR
  sensor.loop();
  #endif

  // Increment mode via console
  bool changeMode = false;
  while (Serial.available()) {
    // discard contents of serial
    changeMode = true;
    Serial.read();
  }
  if (changeMode) {
    char new_mode[10];
    sprintf(new_mode, "%d", (config.mode() + 1) % 7);
    push_event(EVT_MODE_CHANGE, new_mode);
  }

  uint8_t event_type;
  char event_data[EVT_MAX_STR_LEN];
  bool new_event = pop_event(&event_type, event_data);
  if (new_event) {
    switch(config.mode()) {
      case 1:
        mode1_event_handler(event_type, event_data);
        break;
      case 3:
        mode3_event_handler(event_type, event_data);
        break;
      case 4:
        mode4_event_handler(event_type, event_data);
        break;
      case 5:
        mode5_event_handler(event_type, event_data);
        break;
      case 6:
        mode6_event_handler(event_type, event_data);
        break;
    }
    notify_subscribers(event_type, event_data);
  }
#ifdef ENABLE_BLE
  ble.loop();
#endif
}
