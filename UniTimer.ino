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
#define ENABLE_BLE
#define ENABLE_GPS
#define ENABLE_SD
#define ENABLE_SENSOR
#define ENABLE_BUZZER
#define ENABLE_ACCURATE_TIMING
#define DEBUG_GPS

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
#define GPS_FIX 2
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
  pinMode (GPS_FIX, INPUT);

  // SENSOR
#ifdef ENABLE_SENSOR
#ifdef ENABLE_ACCURATE_TIMING
  sensor.setupInterruptHandler(&sensor_interrupt);
  sensor.attach_interrupt();
#endif
#endif

  delay(2000); // wait for serial to connect before starting
  Serial.println("Starting");

  // GPS
#ifdef ENABLE_GPS
  gps.setup(&pps_interrupt);
  // the date_callback will be called on each PPS (1/second).
  register_date_callback(date_callback);
#endif

#ifdef ENABLE_BUZZER
  buzzer.setup();
#endif

#ifdef ENABLE_SD
  sd.setup();
  if (sd.status()) {
    Serial.println("SD Card OK");
    buzzer.success();
  } else {
    Serial.println("SD Card Error");
    buzzer.error();
  }
  delay(1000);
#endif

  config.setup();
  if (config.loadedFromDefault()) {
    Serial.println("Config File not found, loaded defaults");
    buzzer.error();
  } else {
    Serial.println("Config Read Success");
    buzzer.success();
  }
}

void date_callback(byte *hour, byte *minute, byte *second) {
  if (gps.current_time(hour, minute, second)) {
    char value_string[EVT_MAX_STR_LEN];
    snprintf(value_string, EVT_MAX_STR_LEN, "%02d:%02d:%02d", *hour, *minute, *second);
    push_event(EVT_TIME_CHANGE, value_string);
  }
}

void (*mode_setup_method)();
void (*mode_event_handler)(uint8_t event_type, char *event_data);
void (*mode_teardown_method)();

// listen for mode change, and change our internal mode
void mode_change_callback(uint8_t event_type, char *event_data) {
  if (event_type == EVT_MODE_CHANGE) {
    if (mode_teardown_method != NULL) {
      mode_teardown_method();
    }
    if (mode_event_handler != NULL) {
      unregister_subscriber(mode_event_handler);
    }

    config.setMode(atoi(event_data));
    Serial.println("changing Mode to: ");
    Serial.println(config.mode());

    switch(config.mode()) {
      case 1:
        mode_setup_method = NULL;
        mode_event_handler = mode1_event_handler;
        mode_teardown_method = NULL;
        break;
      case 2:
      // no action
        mode_setup_method = NULL;
        mode_event_handler = NULL;
        mode_teardown_method = NULL;
        break;
      case 3:
        mode_setup_method = mode3_setup;
        mode_event_handler = mode3_event_handler;
        mode_teardown_method = mode3_teardown;
        break;
      case 4:
        mode_setup_method = mode4_setup;
        mode_event_handler = mode4_event_handler;
        mode_teardown_method = mode4_teardown;
        break;
      case 5:
        mode_setup_method = mode5_setup;
        mode_event_handler = mode5_event_handler;
        mode_teardown_method = mode5_teardown;
        break;
      case 6:
        mode_setup_method = mode6_setup;
        mode_event_handler = mode6_event_handler;
        mode_teardown_method = mode6_teardown;
        break;
    }

    if (mode_setup_method != NULL) {
      mode_setup_method();
    }
    if (mode_event_handler != NULL) {
      register_subscriber(mode_event_handler);
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

  register_subscriber(&mode_change_callback);
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
    notify_subscribers(event_type, event_data);
  }
#ifdef ENABLE_BLE
  ble.loop();
#endif
}
