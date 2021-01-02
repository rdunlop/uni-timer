// BLE
#include "uni_ble.h"
#include "event_queue.h"
/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

// BLE Library Documentation: https://github.com/nkolban/esp32-snippets/blob/master/Documentation/BLE%20C%2B%2B%20Guide.pdf


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

class FilenameCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // do something because a new value was written.
    Serial.println("Filename Written");
    std::string os;
    os = pCharacteristic->getValue();
    Serial.write(os.c_str());
    Serial.println();
    Serial.print("Filename: ");
    Serial.println(os.c_str());
    push_event(EVT_FILENAME_ENTERED, os.c_str());
  }
};

// empty constructor
UniBle::UniBle()
{

}

void UniBle::setupSensor(BLEService *pService) {
  pSensorCharacteristic = pService->createCharacteristic(
                                         SENSOR_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor to allow subscribing to this characteristic (to support NOTIFY/INDICATE flagging)
  pSensorCharacteristic->addDescriptor(new BLE2902());
  addNameDescriptor(pSensorCharacteristic, "Sensor");
}

void UniBle::setupMode(BLEService *pService) {
  pModeCharacteristic = pService->createCharacteristic(
                                         MODE_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pModeCharacteristic->setCallbacks(new ModeCallback());
  addUtf8Descriptor(pModeCharacteristic);
  addNameDescriptor(pModeCharacteristic, "Mode");
}

void UniBle::setupRacerNumber(BLEService *pService) {
  pRacerNumberCharacteristic = pService->createCharacteristic(
                                         RACER_NUMBER_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pRacerNumberCharacteristic->setCallbacks(new RacerNumberCallback());
  addUtf8Descriptor(pRacerNumberCharacteristic);
  addNameDescriptor(pRacerNumberCharacteristic, "Racer Number");
}

void UniBle::setupResultCount(BLEService *pService) {
  pResultCountCharacteristic = pService->createCharacteristic(
                                         RESULT_COUNT_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pResultCountCharacteristic->addDescriptor(new BLE2902());
  addNameDescriptor(pResultCountCharacteristic, "Cached Result Count");
}

void UniBle::setupCurrentTime(BLEService *pService) {
  pCurrentTimeCharacteristic = pService->createCharacteristic(
                                        CURRENT_TIME_UUID,
                                        BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pCurrentTimeCharacteristic->addDescriptor(new BLE2902());
  addNameDescriptor(pCurrentTimeCharacteristic, "Current Clock Time");
}

void UniBle::setupBuzzer(BLEService *pService) {
  pBuzzerCharacteristic = pService->createCharacteristic(
                                         BUZZER_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pBuzzerCharacteristic->addDescriptor(new BLE2902());
  addNameDescriptor(pBuzzerCharacteristic, "Buzzer");
}

void UniBle::setupFilename(BLEService *pService) {
  pFilenameCharacteristic = pService->createCharacteristic(
                                         FILENAME_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pFilenameCharacteristic->setCallbacks(new FilenameCallback());
  addUtf8Descriptor(pFilenameCharacteristic);
  addNameDescriptor(pFilenameCharacteristic, "Filename");
}

// Callback method which listens to events, and publishes them to the BT data connection
// if they are relevant
void UniBle::bt_notify_callback(uint8_t event_type, char *event_data) {
    switch(event_type) {
    case EVT_SENSOR_CHANGE:
      pSensorCharacteristic->setValue((uint8_t*)event_data, strlen(event_data));
      pSensorCharacteristic->notify();
      break;
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
    case EVT_RACER_NUMBER_ENTERED:
    case EVT_RACER_NUMBER_CLEARED:
      pRacerNumberCharacteristic->setValue((uint8_t*)event_data, strlen(event_data));
      pRacerNumberCharacteristic->notify();
      break;
    case EVT_CACHED_TIME_COUNT:
      pResultCountCharacteristic->setValue((uint8_t*)event_data, strlen(event_data));
      pResultCountCharacteristic->notify();
      break;
    }
}

void UniBle::setup() {
  // BLE SETUP *******************************************************
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32");
  // BLEDevice::setMTU(100);
  pServer = BLEDevice::createServer();
  // Increasing the num_handles to 30 allows for more characteristics
  // the default value (15) https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/src/BLEServer.h#L67
  // prevents > ~5 characteristics from appearing when advertising
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID), 30);

  // set up BLE characteristics
  setupSensor(pService);
  setupMode(pService);
  setupFilename(pService);
  setupBuzzer(pService);
  setupCurrentTime(pService);
  setupRacerNumber(pService);
  setupResultCount(pService);

//  pBatteryCharacteristic->setValue("Hello World says Neil");

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID); // necessary so that the App can identify this device without first connecting
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  // BLE SETUP *******************************************************

  Serial.println("BLE Startup Complete!");
}

void UniBle::loop() {
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

void UniBle::addUtf8Descriptor(BLECharacteristic *characteristic) {
  BLE2904 *format = new BLE2904();
  format->setFormat(BLE2904::FORMAT_UTF8);
  characteristic->addDescriptor(format);
}

void UniBle::addNameDescriptor(BLECharacteristic *characteristic, char *name) {
  BLEDescriptor *nameDescr = new BLEDescriptor(BLEUUID((uint16_t) 0x2901));
  nameDescr->setValue(name);
  characteristic->addDescriptor(nameDescr);
}
