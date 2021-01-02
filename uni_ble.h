#ifndef UNI_BLE_H
#define UNI_BLE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <string.h>

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
// Read the number of results cached on the device, awaiting Racer #
#define RESULT_COUNT_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a3" // R
#define DELETE_RESULT_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a5" // W
#define DUPLICATE_RESULT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a6" // W

class UniBle
{
  public:
    UniBle();
    // methods
    void bt_notify_callback(uint8_t event_type, char *event_data);
    void setup();
    void loop();
  private:
    bool deviceConnected = true;
    bool oldDeviceConnected = false;
    BLEServer* pServer = NULL;
    BLECharacteristic* pModeCharacteristic = NULL;
    BLECharacteristic* pRacerNumberCharacteristic = NULL;
    BLECharacteristic* pFilenameCharacteristic = NULL;
    BLECharacteristic* pBuzzerCharacteristic = NULL;
    BLECharacteristic* pSensorCharacteristic = NULL;
    BLECharacteristic* pCurrentTimeCharacteristic = NULL;
    BLECharacteristic* pResultCountCharacteristic = NULL;
    void setupSensor(BLEService *pService);
    void setupMode(BLEService *pService);
    void setupFilename(BLEService *pService);
    void setupBuzzer(BLEService *pService);
    void setupCurrentTime(BLEService *pService);
    void setupRacerNumber(BLEService *pService);
    void setupResultCount(BLEService *pService);
};

#endif
