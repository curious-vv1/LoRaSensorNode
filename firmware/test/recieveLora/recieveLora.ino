/*

  This example shows how to connect to an EBYTE transceiver
  using an ESP32

  This code is for the receiver

  ESP32 won't allow SoftwareSerial (at least I can't get that lib to work
  so you will just hardwire your EBYTE directly to the Serial2 port

*/

#include "EBYTE.h"

/*
WARNING: IF USING AN ESP32
DO NOT USE THE PIN NUMBERS PRINTED ON THE BOARD
YOU MUST USE THE ACTUAL GPIO NUMBER
*/
#define PIN_RX 16   // Serial2 RX (connect this to the EBYTE Tx pin)
#define PIN_TX 17   // Serial2 TX pin (connect this to the EBYTE Rx pin)

#define PIN_M0 12    
#define PIN_M1 13   
#define PIN_AX 14   

// create the transceiver object, passing in the serial and pins
EBYTE Transceiver(&Serial2, PIN_M0, PIN_M1, PIN_AX);

unsigned long Last;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial.println("Starting Receiver");

  // this init will set the pinModes for you
  Transceiver.init();

  // all these calls are optional but shown to give examples of what you can do
  // Serial.println(Transceiver.GetAirDataRate());
  // Serial.println(Transceiver.GetChannel());
  // Transceiver.SetAddressH(1);
  // Transceiver.SetAddressL(1);
  // int Chan = 15;
  // Transceiver.SetChannel(Chan);
  // Transceiver.SetPullupMode(1);
  // Transceiver.SaveParameters(PERMANENT);

  // you can print all parameters and is good for debugging
  // if your units will not communicate, print the parameters
  // for both sender and receiver and make sure air rates, channel
  // and address is the same
  Transceiver.PrintParameters();
}

void loop() {
  // Buffer to store the received message
  char message[32];

  // Check if data is available to read
  if (Transceiver.available()) {
    // Read the incoming message
    Transceiver.GetStruct(message, sizeof(message));

    // Print the received message
    Serial.print("Received: ");
    Serial.println(message);

    // Update the last received time
    Last = millis();
  } else {
    // If no data is received, print "Searching: " every second
    if ((millis() - Last) > 1000) {
      Serial.println("Searching: ");
      Last = millis();
    }
  }
}
