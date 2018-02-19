#include <Adafruit_MCP9808.h>
#include <Adafruit_SGP30.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#ifdef ESP32
#include "Esp32.h"
#endif

#ifdef ESP8266
#include "Esp8266.h"
#endif

#define MCP9808_CALIBRATION_OFFSET -2
Adafruit_SGP30 sgp;
Adafruit_MCP9808 tempsensor;

const char *AWS_endpoint = "a11g3hu5pi5k6j.iot.us-west-2.amazonaws.com"; //MQTT broker ip
const char *ssid = "CHANGEME";
const char *password = "CHANGEME";

StaticJsonBuffer<200> jsonBuffer;
char jsonData[200];
WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, espClient); //set  MQTT port number to 8883 as per //standard
int counter = 0;

void setup() {
    Serial.begin(115200);

#ifdef SGP30
    Serial.println("SGP30 test");

    if (!sgp.begin()) {
        Serial.println("Sensor not found :(");
        while (1);
    }
    Serial.print("Found SGP30 serial #");
    Serial.print(sgp.serialnumber[0], HEX);
    Serial.print(sgp.serialnumber[1], HEX);
    Serial.println(sgp.serialnumber[2], HEX);

    // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
    //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
#endif

#ifdef MCP9808
    Serial.println("MCP9808 demo");
    // Make sure the sensor is found, you can also pass in a different i2c
    // address with tempsensor.begin(0x19) for example
    if (!tempsensor.begin()) {
        Serial.println("Couldn't find MCP9808!");
        while (1);
    }
#endif

    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // IoT
    loadClientCerts(espClient);
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("airstation")) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    JsonObject &root = jsonBuffer.createObject();
    JsonObject &state = root.createNestedObject("state").createNestedObject("reported");

#ifdef MCP9808
    Serial.println("wake up MCP9808.... "); // wake up MSP9808 - power consumption ~200 mikro Ampere
    tempsensor.wake();   // wake up, ready to read!
#endif

#ifdef SGP30
    if (!sgp.IAQmeasure()) {
        Serial.println("Measurement failed");
        return;
    }
    Serial.print("TVOC ");
    Serial.print(sgp.TVOC);
    state["TVOC"] = sgp.TVOC;
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");
    Serial.print(sgp.eCO2);
    state["eCO2"] = sgp.eCO2;
    Serial.println(" ppm");

    counter++;
    if (counter == 30) {
        counter = 0;

        uint16_t TVOC_base, eCO2_base;
        if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
            Serial.println("Failed to get baseline readings");
            return;
        }
        Serial.print("****Baseline values: eCO2: 0x");
        Serial.print(eCO2_base, HEX);
        Serial.print(" & TVOC: 0x");
        Serial.println(TVOC_base, HEX);

        state["eCO2_base"] = eCO2_base;
        state["TVOC_base"] = TVOC_base;
    }
#endif

#ifdef MCP9808
    // Read and print out the temperature, then convert to *F
    float c = tempsensor.readTempC() + MCP9808_CALIBRATION_OFFSET;
    float f = c * 9.0 / 5.0 + 32;
    Serial.print("Temp: ");
    Serial.print(c);
    Serial.print("*C\t");
    Serial.print(f);
    Serial.println("*F");

    state["Temp"] = c;

    Serial.println("Shutdown MCP9808.... ");
    tempsensor.shutdown(); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
#endif

    root.printTo(jsonData);
    client.publish("$aws/things/airstation/shadow/update", jsonData);
    jsonBuffer.clear();

    unsigned long lastMillis = millis();
    while (millis() - lastMillis < 30000) {
        client.loop();
        delay(1000);
    }
}
