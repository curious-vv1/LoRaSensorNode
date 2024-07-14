#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <Adafruit_MLX90614.h>
#include <LoRa.h>
#include <SPI.h>

// SSID and Password of your WiFi router
const char* ssid = "motog52";
const char* password = "12ka442ka1";

PulseOximeter pox;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define SDA_PIN_MAX30100 21
#define SCL_PIN_MAX30100 22

#define SDA_PIN_MLX90614 4
#define SCL_PIN_MLX90614 27
#define MLX90614_ADDR 0x5A

#define GSR_PIN 32

#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

TwoWire WireMAX30100(0);
TwoWire WireMLX90614(1);

WebServer server(80);

float heartRate = 0;
float spO2 = 0;
float ambientTemp = 0;
float objectTemp = 0;
int gsrValue = 0;

uint32_t tsLastReport = 0;

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Sensor Data</title>
    <style>
        .circle {
            width: 150px;
            height: 150px;
            border-radius: 50%;
            background-color: #f0f0f0;
            display: inline-flex;
            justify-content: center;
            align-items: center;
            margin: 10px;
            font-family: Arial, sans-serif;
            flex-direction: column;
        }
        .label {
            font-weight: bold;
            margin-bottom: 5px;
        }
    </style>
    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('heartRate').innerHTML = data.heartRate.toFixed(1);
                    document.getElementById('spO2').innerHTML = data.spO2.toFixed(1);
                    document.getElementById('ambientTemp').innerHTML = data.ambientTemp.toFixed(1);
                    document.getElementById('objectTemp').innerHTML = data.objectTemp.toFixed(1);
                    document.getElementById('gsrValue').innerHTML = data.gsrValue;
                });
        }
        setInterval(updateData, 1000);
    </script>
</head>
<body>
    <center>
        <h1>Sensor Data</h1>
        <div class="circle">
            <div class="label">Heart Rate</div>
            <div id="heartRate">0</div>
        </div>
        <div class="circle">
            <div class="label">SpO2</div>
            <div id="spO2">0</div>
        </div>
        <div class="circle">
            <div class="label">Ambient Temp</div>
            <div id="ambientTemp">0</div>
        </div>
        <div class="circle">
            <div class="label">Object Temp</div>
            <div id="objectTemp">0</div>
        </div>
        <div class="circle">
            <div class="label">GSR Value</div>
            <div id="gsrValue">0</div>
        </div>
    </center>
</body>
</html>
)=====";

void handleRoot() {
    server.send(200, "text/html", MAIN_page);
}

void handleData() {
    String json = "{";
    json += "\"heartRate\":" + String(heartRate) + ",";
    json += "\"spO2\":" + String(spO2) + ",";
    json += "\"ambientTemp\":" + String(ambientTemp) + ",";
    json += "\"objectTemp\":" + String(objectTemp) + ",";
    json += "\"gsrValue\":" + String(gsrValue);
    json += "}";
    server.send(200, "application/json", json);
}

void onBeatDetected() {
    Serial.println("Beat!");
}

bool initializePulseOximeter() {
    Serial.print("Initializing pulse oximeter...");
    if (!pox.begin()) {
        Serial.println("FAILED");
        return false;
    } else {
        Serial.println("SUCCESS");
        pox.setOnBeatDetectedCallback(onBeatDetected);
        return true;
    }
}

bool initializeMLX90614() {
    Serial.print("Initializing MLX90614...");
    WireMLX90614.begin(SDA_PIN_MLX90614, SCL_PIN_MLX90614);
    if (mlx.begin(MLX90614_ADDR, &WireMLX90614)) {
        delay(40);
        Serial.println("SUCCESS");
        return true;
    } else {
        Serial.println("FAILED");
        return false;
    }
}

bool initializeLoRa() {
    Serial.print("Initializing LoRa...");
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("FAILED");
        return false;
    }
    LoRa.setSyncWord(0xA5);
    Serial.println("SUCCESS");
    return true;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    if (!initializePulseOximeter() || !initializeMLX90614() || !initializeLoRa()) {
        Serial.println("Initialization failed. Halting program.");
        while (true);
    }

    pinMode(GSR_PIN, INPUT);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    pox.update();

    if (millis() - tsLastReport > 1000) {  // Send data every second
        heartRate = pox.getHeartRate();
        spO2 = pox.getSpO2();
        ambientTemp = mlx.readAmbientTempC();
        objectTemp = mlx.readObjectTempC();
        gsrValue = analogRead(GSR_PIN);

        // Prepare data string
        String data = "HR:" + String(heartRate) + 
                      ",SpO2:" + String(spO2) + 
                      ",AmbT:" + String(ambientTemp) + 
                      ",ObjT:" + String(objectTemp) + 
                      ",GSR:" + String(gsrValue);

        // Send data via LoRa
        LoRa.beginPacket();
        LoRa.print(data);
        LoRa.endPacket();

        // Print data to Serial for debugging
        Serial.println("Sending LoRa packet: " + data);

        tsLastReport = millis();
    }
}