#include <WiFi.h>
#include <WebServer.h>
#include <LoRa.h>

// SSID and Password of your WiFi router
const char* ssid = "motog52";
const char* password = "12ka442ka1";

// LoRa pins
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

// Global variables to store sensor data
float heartRate = 0;
float spO2 = 0;
float ambientTemp = 0;
float objectTemp = 0;
int gsrValue = 0;

// Declare a global object variable from the WebServer class
WebServer server(80); // Server on port 80

// HTML webpage contents
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

void setup() {
    Serial.begin(115200);
    
    // Initialize LoRa
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa initialization failed. Check your connections.");
        while (true);
    }
    LoRa.setSyncWord(0xA5);
    
    // Connect to WiFi
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
    
    // Set up web server routes
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    
    // Check if there's any incoming LoRa packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        String message = "";
        while (LoRa.available()) {
            message += (char)LoRa.read();
        }
        
        // Parse the received data
        if (message.startsWith("HR:")) {
            int index = message.indexOf(",SpO2:");
            heartRate = message.substring(3, index).toFloat();
            
            index = message.indexOf(",AmbT:");
            spO2 = message.substring(message.indexOf("SpO2:") + 5, index).toFloat();
            
            int index2 = message.indexOf(",ObjT:");
            ambientTemp = message.substring(index + 6, index2).toFloat();
            
            index = message.indexOf(",GSR:");
            objectTemp = message.substring(index2 + 6, index).toFloat();
            
            gsrValue = message.substring(index + 5).toInt();
            
            Serial.println("Received LoRa packet: " + message);
        }
    }
}