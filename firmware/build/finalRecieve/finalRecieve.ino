#include <LoRa.h>
#include <SPI.h>

#define ss 5
#define rst 14
#define dio0 2

void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver for Sensor Data");

  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(433E6))
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");
}

void loop() 
{
  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    String LoRaData = "";
    while (LoRa.available())
    {
      LoRaData += (char)LoRa.read();
    }

    Serial.println("Received packet: '" + LoRaData + "'");
    Serial.print("RSSI: ");
    Serial.println(LoRa.packetRssi());

    // Parse and print individual sensor readings
    parseAndPrintSensorData(LoRaData);
  }
}

void parseAndPrintSensorData(String data)
{
  int startIndex = 0;
  int endIndex = data.indexOf(',');
  while (endIndex != -1)
  {
    String pair = data.substring(startIndex, endIndex);
    printPair(pair);
    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
  }
  // Print the last pair
  printPair(data.substring(startIndex));
}

void printPair(String pair)
{
  int colonIndex = pair.indexOf(':');
  if (colonIndex != -1)
  {
    String key = pair.substring(0, colonIndex);
    String value = pair.substring(colonIndex + 1);
    Serial.print(key + ": ");
    Serial.println(value);
  }
}