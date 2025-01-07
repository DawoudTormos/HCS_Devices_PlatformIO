#include <Arduino.h>

#include <EEPROM.h>
#include <WebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LED_BUILTIN 2
#define RX_PIN 16
#define TX_PIN 17
// Global variables
WebServer server(80);
char ssid[32] = "hp-printer_8B7D";
char password[32] = "Router-Pass12345";
bool connectionMode = false;
String basURL = "http://192.168.77.100:8080/";

String ID = "10006";
String token = "dJVprAAXkMXWuPjqgDSPL5E47COsCt3ERyT5Z39g5qprYQvsoB19PS7FIERxOzpj";


String cmd = "";
char *SSID = "Zahi";
char *PASS = "test12345678";
WiFiClient wifiClient;

void setup_inner();
void readCredentials();
void saveCredentials(const char* newSSID, const char* newPassword);

void postDataToServer(String url, String data) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.println("POST TO " + url + " with " + data);
        http.begin(wifiClient, url); // Use updated API with WiFiClient
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Content-Length", String(data.length())); 
        http.addHeader("User-Agent", "ESP32/Arduino");
        http.addHeader("Host", "a95b9573e08efcb405557dc38bdc3190.serveo.net");
        http.addHeader("Connection", "keep-alive");
        http.addHeader("Accept", "*/*");

        int httpCode = http.POST(data); // POST request
        if (httpCode > 0) {
            Serial.println("POSTING to " + url + "...");
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println("Response: " + payload);
            } else {
                Serial.printf("HTTP POST failed. Code: %d\n", httpCode);
            }
        } else {
            Serial.printf("POST failed. Error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("Wi-Fi not connected.");
    }
}

// Setup function
void setup() {
    Serial.begin(115200); // Set baud rate for serial communication
      Serial2.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);

    pinMode(LED_BUILTIN , OUTPUT);
    digitalWrite(LED_BUILTIN , HIGH);

    setup_inner();
}

// Main loop
void loop() {
    if (connectionMode) {
        server.handleClient();
    } else {
        if (Serial2.available() > 0) {


            String input = Serial2.readStringUntil('\n');
            input.trim();

            Serial.println(input);
            String JSONData = ID + "," + token + ","+ input;
            String URL = basURL + "sendSensorData";
            Serial.println("POST TO " + URL + " with " + JSONData);

            postDataToServer(URL, JSONData);
            Serial.println("END");


            digitalWrite(LED_BUILTIN , LOW);
            delay(500);
            digitalWrite(LED_BUILTIN , HIGH);
        }
    }
}

// Connection mode setup
void setupConnectionMode() {
    WiFi.softAP("ESP_Config", "password123");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Point IP: ");
    Serial.println(IP);

    server.on("/", []() {
        String html = R"(
            <html>
            <body>
            <h1>Wi-Fi Configuration</h1>
            <form action="/save" method="POST">
                SSID: <input type="text" name="ssid"><br>
                Password: <input type="password" name="password"><br>
                <input type="submit" value="Save">
            </form>
            </body>
            </html>
        )";
        server.send(200, "text/html", html);
    });

    server.on("/save", []() {
        String newSSID = server.arg("ssid");
        String newPassword = server.arg("password");
        saveCredentials(newSSID.c_str(), newPassword.c_str());
        server.send(200, "text/html", "Credentials saved. Restart device.");
    });

    server.begin();
    Serial.println("Web server started. Waiting for configuration...");
}

// Normal mode setup
void setupNormalMode() {
    readCredentials();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print(ssid);
    Serial.println(password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println("\nConnected to Wi-Fi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup_inner() {
    while (Serial2.available() == 0) {}
    
    if (Serial2.available() > 0) {
        String input = Serial2.readStringUntil('\n');
        input.trim();

        digitalWrite(LED_BUILTIN , LOW);
        delay(500);
        digitalWrite(LED_BUILTIN , HIGH);

        if (input == "AP") {
        digitalWrite(LED_BUILTIN , LOW);
        delay(200);
        digitalWrite(LED_BUILTIN , HIGH);
        delay(200);
                digitalWrite(LED_BUILTIN , LOW);
        delay(200);
        digitalWrite(LED_BUILTIN , HIGH);
        delay(200);
                digitalWrite(LED_BUILTIN , LOW);
        delay(200);
        digitalWrite(LED_BUILTIN , HIGH);
        delay(200);
            Serial.println("Entering Connection Mode.");
            connectionMode = true;
            setupConnectionMode();
        } else if (input == "NORMAL") {
            Serial.println("Entering Normal Mode.");
            connectionMode = false;
            setupNormalMode();
        } else {//wait for next serial input
            setup_inner();
        }
    }
}

// Function to read credentials from EEPROM
void readCredentials() {
    EEPROM.begin(512);
    for (int i = 0; i < 32; i++) {
        ssid[i] = EEPROM.read(i);
        password[i] = EEPROM.read(i + 32);
    }
    ssid[31] = '\0';
    password[31] = '\0';
    EEPROM.end();
}

// Function to save credentials to EEPROM
void saveCredentials(const char* newSSID, const char* newPassword) {
    EEPROM.begin(512);
    for (int i = 0; i < 32; i++) {
        EEPROM.write(i, newSSID[i]);
        EEPROM.write(i + 32, newPassword[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}
