#include "uni_radio.h"
#include "recording.h" // for log()

void UniRadio::setup(bool enabled, uint8_t radio_id, uint8_t radio_target_id) {
  // pinMode(RFM95_RST, OUTPUT);
  // digitalWrite(RFM95_RST, HIGH);

  _enabled = enabled;
  if (!_enabled) {
    // do not try to initialize radio.
    return;
  }
  // manual reset
  // digitalWrite(RFM95_RST, LOW);
  // delay(10);
  // digitalWrite(RFM95_RST, HIGH);
  // delay(10);

  if (!_initialized) {
    _initialized = true;
    if (!_rf95.init()) {
      _status = false;
      Serial.println("init failed");
    } else {
      _status = true;
    }

    _rf95.setFrequency(915.0); // TBD

    _rf95.setModemConfig(RH_RF95::ModemConfigChoice::Bw125Cr48Sf4096);

    _rf95.setTxPower(23);
  }

  if (radio_id != 0) {
    _rf95.setThisAddress(radio_id);
    _rf95.setHeaderFrom(radio_id);
    _rf95.setPromiscuous(false);
  } else {
    _rf95.setThisAddress(RH_BROADCAST_ADDRESS);
    _rf95.setHeaderFrom(RH_BROADCAST_ADDRESS);
    _rf95.setPromiscuous(true);
  }

  if (radio_target_id != 0) {
    _rf95.setHeaderTo(radio_target_id);
  } else {
    _rf95.setHeaderTo(RH_BROADCAST_ADDRESS);
  }
}

// Returns true on success
bool UniRadio::status() {
  if (!_enabled) { return false; }

  return _status;
}

// Sends a message, and then returns
// Returns true if was sent successfully
// Returns false if no message received
//
// NOTE: this honors the radio configuration/id set on this unit
bool UniRadio::senderTest() {
  if (!_enabled) { return false; }

  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  char data[17];
  snprintf(data, 16, "%ld HelloWorld", millis());
  log("sending ~16 char message");
  if (_rf95.send((uint8_t *)data, strlen(data))) {
    log("send() complete");

    if (_rf95.waitPacketSent()) {
      log("waitPacketSent() complete");
      return true;
    } else {
      log("waitPacketSent() failed");
    }
  } else {
    log("Send() failed");
  }
  return false;
}

// Designed to be run in a loop, so that it immediately receives/displays the message
// Upon receipt
//
// Returns true if there's a new message, and returns the message and rssi
// Returns false if there's no new message
bool UniRadio::receiverTest( char *message_received, int *rssi_received) {
  if (_rf95.waitAvailableTimeout(1000))
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

#define MAX_MESSAGES 20
#define MAX_MESSAGE_LENGTH 25
// %d,,%02d,%02d,%03d,%d

// - racer number (1-4 digits)
// - DQ, DNF, or empty (if valid)
// - minute-of-day (0-2880)
// - second-of-minute (00-59)
// - millisecond-of-minute (000-999)
// - penalties (0-1) - was this an early-start (on a count-down-based start config)
// max total length: 4 + 1 + 1 + 4 + 1 + 2 + 1 + 3 + 1 + 1 = 19

char results_to_transmit[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
int tx_results_count = 0;
// Oldest result is at index 0
// Newest result is at index [tx_results_count - 1]

// if there is a message to be sent
// check to see if the radio is available
// And send the message
void UniRadio::loop()
{
  if (tx_results_count > 0) {
    // There is a message to send
    if (_rf95.mode() == RHGenericDriver::RHModeIdle) { // ? should it be != RHModeTx ?
      // the radio is available
      if (_rf95.send((uint8_t *)results_to_transmit[0], strlen(results_to_transmit[0]) + 1)) {
        // send was successful
        // Drop the sent entry
        for (int i = 1; i < tx_results_count; i++) {
          memcpy(results_to_transmit[i - 1], results_to_transmit[i], MAX_MESSAGE_LENGTH);
        }
        memset(results_to_transmit[tx_results_count - 1], 0, MAX_MESSAGE_LENGTH);
        tx_results_count -= 1;
      } else {
        log("RADIO - Error sending via radio");
      }
    }
  }
}

// queue a message for sending
// returns true if queued
// returns false if full
bool UniRadio::queueToSend(char *message) {
  if (!status()) {
    Serial.println("RADIO - fail");
    return false;
  }
  if (tx_results_count >= MAX_MESSAGES) {
    Serial.println("RADIO - queuing full");
    return false;
  }

  strcpy(results_to_transmit[tx_results_count], message);
  tx_results_count += 1;
  Serial.println("RADIO - queued");
  return true;
}

int UniRadio::queueSize() {
  return tx_results_count;
}

void UniRadio::displayQueue() {
  Serial.print("TX Queue: ");
  Serial.print(tx_results_count);
  Serial.println(" messages");
  for (int i = 0; i < MAX_MESSAGES; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(results_to_transmit[i]);
  }
}

bool UniRadio::messageAvailable() {
  return _rf95.available();
}

bool UniRadio::receive(uint8_t *message, uint8_t *message_length) {
  if (!_rf95.available()) {
    return false;
  }
  if (_rf95.recv(message, message_length)) {
    Serial.println("received: ");
    Serial.println((char*)message);
    return true;
  } else {
    Serial.println("Receive failed");
    return false;
  }
}
