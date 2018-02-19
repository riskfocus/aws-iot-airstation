//
// Created by PVeretennikovs on 19-Feb-18.
//

#ifndef AWS_IOT_AIRSTATION_ESP32_H
#define AWS_IOT_AIRSTATION_ESP32_H

#include <SPIFFS.h>
#include <WiFiClientSecure.h>

bool loadClientCerts(WiFiClientSecure espClient) {
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return false;
    }

    Serial.print("Heap: ");
    Serial.println(ESP.getFreeHeap());

    // Load certificate file
    File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt with your uploaded file name
    if (!cert) {
        Serial.println("Failed to open cert file");
        return false;
    } else
        Serial.println("Success to open cert file");

    auto *certData = (char *) malloc(cert.size());;
    cert.readBytes(certData, cert.size());
    espClient.setCertificate(certData);

    // Load private key file
    File private_key = SPIFFS.open("/private.der", "r"); //replace private with your uploaded file name
    if (!private_key) {
        Serial.println("Failed to open private cert file");
        return false;
    } else
        Serial.println("Success to open private cert file");

    auto *keyData = (char *) malloc(private_key.size());;
    private_key.readBytes(keyData, private_key.size());
    espClient.setPrivateKey(keyData);

    return true;
}

#endif //AWS_IOT_AIRSTATION_ESP32_H
