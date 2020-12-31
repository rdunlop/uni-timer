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
#define ENABLE_GPS
#define ENABLE_SD
#define ENABLE_SENSOR
#define ENABLE_BUZZER
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "event_queue.h"
#include "subscribers.h"

/* *********************** Includes *********************************** */
// - SENSOR
#ifdef ENABLE_GPS
#include "uni_gps.h"
#endif
// - SD Card
#ifdef ENABLE_SD
#include "uni_sd.h"
#endif
// - BUZZER
#ifdef ENABLE_BUZZER
#include "uni_buzzer.h"
#endif
#ifdef ENABLE_SENSOR
#include "uni_sensor.h"
#endif
// - BUTTON

#include "modes.h"
#include "recording.h"
#include "uni_config.h"
#include "accurate_timing.h"

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

// SD
#ifdef ENABLE_SD
UniSd sd(
  SD_SPI_CHIP_SELECT_OUTPUT);
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

// GLOBAL STATE MANAGEMENT
UniConfig config; // No arguments for constructor, no parentheses
Config _config;

// NEW HEADER FILE
void date_callback(byte *hour, byte *minute, byte *second);

/******** ***********************************(set up)*** *************** **********************/
void main_setup () {
  // Common
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  // SENSOR
#ifdef ENABLE_SENSOR
  sensor.setup(&sensor_interrupt);
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

  if (config.readConfig(&_config)) {
    Serial.println("Config Read Success");
  } else {
    // Default Config, as the config file is not found
    _config.mode = 1;
    strncpy(_config.filename, "/results.txt", 40);
    config.writeConfig(&_config);
  }
  // the date_callback will be called on each PPS (1/second).
  register_date_callback(date_callback);
#ifdef ENABLE_SENSOR
  sensor.attach_interrupt();
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


/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

// BLE Library Documentation: https://github.com/nkolban/esp32-snippets/blob/master/Documentation/BLE%20C%2B%2B%20Guide.pdf


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

// This is the Service that we advertise to the BLE application
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

// Characteristics

// Which configuration mode are we in, and allow the user to change the mode
#define MODE_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26a9" // R/W
// Read or write the active racer number
#define RACER_NUMBER_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26aa" // R/W
// Read or write the filename to write the results
#define FILENAME_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26a8" // R/W
// Read the state of the buzzer
#define BUZZER_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26a1" // R
// Read the state of the sensor
#define SENSOR_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26a2" // R
// Read the current time from the GPS (note: notification is only 1/second)
#define CURRENT_TIME_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a0" // R
// Read the number of results ?????
#define NUM_RESULTS_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a3" // R
#define STORE_RESULT_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a4" // W
#define DELETE_RESULT_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a5" // W
#define DUPLICATE_RESULT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a6" // W

BLEServer* pServer = NULL;
BLECharacteristic* pModeCharacteristic = NULL;
BLECharacteristic* pRacerNumberCharacteristic = NULL;
BLECharacteristic* pFilenameCharacteristic = NULL;
BLECharacteristic* pBuzzerCharacteristic = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
BLECharacteristic* pCurrentTimeCharacteristic = NULL;

// header
// header

bool deviceConnected = true;
bool oldDeviceConnected = false;

#include <string.h>

class ModeCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // do something because a new value was written.
    Serial.println("Mode Written");
    std::string os;
    os = pCharacteristic->getValue();
    Serial.write(os.c_str());
    Serial.println();
    int num = atoi(os.c_str());
    Serial.print("New Mode: ");
    Serial.println(num);
    push_event(EVT_MODE_CHANGE, os.c_str());
  }
};


class RacerNumberCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // do something because a new value was written.
    Serial.println("Racer Number Written");
    std::string os;
    os = pCharacteristic->getValue();
    Serial.write(os.c_str());
    Serial.println();
    int num = atoi(os.c_str());
    Serial.print("Racer Number: ");
    Serial.println(num);
    push_event(EVT_RACER_NUMBER_ENTERED, os.c_str());
  }
};

void setupSensor(BLEService *pService) {
  pSensorCharacteristic = pService->createCharacteristic(
                                         SENSOR_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor to allow subscribing to this characteristic (to support NOTIFY/INDICATE flagging)
  pSensorCharacteristic->addDescriptor(new BLE2902());
}

void setupMode(BLEService *pService) {
  pModeCharacteristic = pService->createCharacteristic(
                                         MODE_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pModeCharacteristic->setCallbacks(new ModeCallback());
}

void setupRacerNumber(BLEService *pService) {
  pRacerNumberCharacteristic = pService->createCharacteristic(
                                         RACER_NUMBER_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pRacerNumberCharacteristic->setCallbacks(new RacerNumberCallback());
}

void setupCurrentTime(BLEService *pService) {
  pCurrentTimeCharacteristic = pService->createCharacteristic(
                                        CURRENT_TIME_UUID,
                                        BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_INDICATE
                                       );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor to allow subscribing to this characteristic (to support NOTIFY/INDICATE flagging)
  pCurrentTimeCharacteristic->addDescriptor(new BLE2902());
}

void setupBuzzer(BLEService *pService) {
  pBuzzerCharacteristic = pService->createCharacteristic(
                                         BUZZER_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                       );
}

// Callback method which listens to events, and publishes them to the BT data connection
// if they are relevant
void bt_notify_callback(uint8_t event_type, char *event_data) {
    switch(event_type) {
    case EVT_BUZZER_CHANGE:
      pBuzzerCharacteristic->setValue((uint8_t*)event_data, strlen(event_data));
      pBuzzerCharacteristic->notify();
      break;
    case EVT_TIME_CHANGE:
      pCurrentTimeCharacteristic->setValue((uint8_t*)event_data, strlen(event_data));
      pCurrentTimeCharacteristic->notify();
      break;
    case EVT_MODE_CHANGE:
      pModeCharacteristic->setValue((uint8_t*)event_data, strlen(event_data));
      pModeCharacteristic->notify();
      break;
    }
}

// listen for mode change, and change our internal mode
void mode_callback(uint8_t event_type, char *event_data) {
  if (event_type == EVT_MODE_CHANGE) {
    int previousMode = _config.mode;
    if (previousMode == 4) {
      mode4_teardown();
    } else if (previousMode == 5) {
      mode5_teardown();
    } else if (previousMode == 6) {
      mode6_teardown();
    }

    _config.mode = atoi(event_data);
    config.writeConfig(&_config);
    Serial.println("changing Mode to: ");
    Serial.println(_config.mode);

    if (_config.mode == 4) {
      mode4_setup();
    } else if (_config.mode == 5) {
      mode5_setup();
    } else if(_config.mode == 6) {
      mode6_setup();
    }

  }
}

// This method is run once-and-only-once when the device
// is first powered on.
// It does Self-test, and then configures the device in the
// appropriate mode (if possible)
// After this method finishes, the loop() method runs over and over again.
void setup() {
  Serial.begin(115200);
  main_setup();

  // Extract BLE SETUP *******************************************************
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // set up BLE characteristics
  setupSensor(pService);
  setupMode(pService);
  setupBuzzer(pService);
  setupCurrentTime(pService);
  setupRacerNumber(pService);

//  pBatteryCharacteristic->setValue("Hello World says Neil");

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID); // necessary so that the App can identify this device without first connecting
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  // BLE SETUP *******************************************************

  Serial.println("Startup Complete!");

  register_subscriber(&bt_notify_callback);
  register_subscriber(&mode_callback);
}

// once the main() method completes,
// this method will run over and over again, as quickly
// and frequently as possible.
void loop() {
  gps.readData();
  gps.printPeriodically();
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
    sprintf(new_mode, "%d", (_config.mode + 1) % 7);
    push_event(EVT_MODE_CHANGE, new_mode);
  }

  uint8_t event_type;
  char event_data[EVT_MAX_STR_LEN];
  bool new_event = pop_event(&event_type, event_data);
  if (new_event) {
    switch(_config.mode) {
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

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    Serial.println("Connecting BLE device");
    oldDeviceConnected = deviceConnected;
  }
}
