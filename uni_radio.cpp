#include "uni_radio.h"

void UniRadio::setup() {
  // pinMode(RFM95_RST, OUTPUT);
  // digitalWrite(RFM95_RST, HIGH);

  pinMode(_interrupt, INPUT);

  Serial.println("Arduino LoRa TX Test!");
  // manual reset
  // digitalWrite(RFM95_RST, LOW);
  // delay(10);
  // digitalWrite(RFM95_RST, HIGH);
  // delay(10);

  if (!_rf95.init()) {
    _status = false;
    Serial.println("init failed");
  } else {
    _status = true;
  }

  _rf95.setFrequency(915.0); // TBD
}

// Returns true on success
bool UniRadio::status() {
  return _status;
}

void UniRadio::loop()
{
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  uint8_t data[] = "Hello World!";
  _rf95.send(data, sizeof(data));

  _rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (_rf95.waitAvailableTimeout(3000))
  {
    // Should be a reply message for us now
    if (_rf95.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
      //      Serial.print("RSSI: ");
      //      Serial.println(rf95.lastRssi(), DEC);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
  }
  delay(400);
}
