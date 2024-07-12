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

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);

  Serial.println("Starting Transmitter");

  // this init will set the pinModes for you
  Serial.println(Transceiver.init());

  // all these calls are optional but shown to give examples of what you can do
  // Serial.println(Transceiver.GetAirDataRate());
  // Serial.println(Transceiver.GetChannel());
  // Transceiver.SetAddressH(1);
  // Transceiver.SetAddressL(1);
  // int Chan = 15;
  // Transceiver.SetChannel(Chan);
  // save the parameters to the unit,
  // Transceiver.SaveParameters(PERMANENT);

  // you can print all parameters and is good for debugging
  // if your units will not communicate, print the parameters
  // for both sender and receiver and make sure air rates, channel
  // and address is the same
  Transceiver.PrintParameters();
}

void loop() {
  // Send the message "Hello"
  const char* message = "Hello";
  Transceiver.SendStruct(message, strlen(message));

  // Let the user know something was sent
  Serial.print("Sending: ");
  Serial.println(message);
  delay(1000);
}
