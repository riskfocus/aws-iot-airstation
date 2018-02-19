//
// Created by PVeretennikovs on 19-Feb-18.
//

#ifndef AWS_IOT_AIRSTATION_ESP8266_H
#define AWS_IOT_AIRSTATION_ESP8266_H

#include <FS.h>
#include <ESP8266Wifi.h>

bool loadClientCerts(WiFiClientSecure espClient) {
    // IoT
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return false;
    }

    Serial.print("Heap: ");
    Serial.println(ESP.getFreeHeap());

    // Load certificate file
    File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
    if (!cert) {
        Serial.println("Failed to open cert file");
        return false;
    } else
        Serial.println("Success to open cert file");

    delay(1000);

    if (espClient.loadCertificate(cert))
        Serial.println("cert loaded");
    else
        Serial.println("cert not loaded");

    // Load private key file
    File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
    if (!private_key) {
        Serial.println("Failed to open private cert file");
        return false;
    } else
        Serial.println("Success to open private cert file");

    delay(1000);

    if (espClient.loadPrivateKey(private_key))
        Serial.println("private key loaded");
    else {
        Serial.println("private key not loaded");
        return false;
    }

    return true;
}

#endif //AWS_IOT_AIRSTATION_ESP8266_H
