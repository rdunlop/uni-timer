#include "uni_radio.h"
#include "recording.h" // for log()

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

// Sends a message, and waits for a reply for 3 seconds
// Returns true upon reply receipt, the returns message and RSSI upon receipt
// Returns false if no message received
bool UniRadio::senderTest(char *message_received, int *rssi_received) {
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  char data[220];
  snprintf(data, 16, "%d HelloWorld", millis());
  log("sending ~20 char message");
  _rf95.send(data, strlen(data));
  log("send() complete");

  _rf95.waitPacketSent();
  log("waitPacketSent() complete");
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (_rf95.waitAvailableTimeout(3000))
  {
    // Should be a reply message for us now
    if (_rf95.recv(buf, &len))
   {
     log("got reply: ");
     log((char*)buf);
     Serial.print("RSSI: ");
     Serial.println(_rf95.lastRssi(), DEC);
     strncpy(message_received, (char *)buf, 15); // copy first 15 chars
     message_received[15] = 0; // null-terminate the response message in the 16th char
     *rssi_received = _rf95.lastRssi();
     return true;
    }
    else
    {
      Serial.println("recv failed");
      return false;
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
    return false;
  }
}

// Designed to be run in a loop, so that it immediately receives/displays the message
// Upon receipt
//
// Returns true if there's a new message, and returns the message and rssi
// Returns false if there's no new message
bool UniRadio::receiverTest( char *message_received, int *rssi_received) {
  if (_rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (_rf95.recv(buf, &len))
    {
      // digitalWrite(led, HIGH);
//      RH_RF95::printBuffer("request: ", buf, len);
     log("received: ");
     log((char*)buf);
     Serial.print("RSSI: ");
     Serial.println(_rf95.lastRssi(), DEC);
     strncpy(message_received, (char *)buf, 15);
     message_received[15] = 0; // null-terminate the response message
     *rssi_received = _rf95.lastRssi();

      // Send a reply
      uint8_t data[] = "And hello to you";
      _rf95.send(data, sizeof(data));
      _rf95.waitPacketSent();
      log("Sent a reply");
       // digitalWrite(led, LOW);
      return true;
    }
    else
    {
      Serial.println("recv failed");
      return false;
    }
  } else {
    return false;
  }
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
