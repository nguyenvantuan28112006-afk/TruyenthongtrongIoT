#include <Arduino.h>
#include <SoftwareSerial.h>

#define LED_ARDUINO 6

// RX = D10
// TX = D11
SoftwareSerial ESPSerial(10, 11);  

void setup() {

    pinMode(LED_ARDUINO, OUTPUT);
    //
    digitalWrite(LED_ARDUINO, LOW);

    // Serial Monitor trên máy tính
    Serial.begin(9600);
    // t

    // UART với ESP32
    ESPSerial.begin(9600);
    // xét tốc độ giữa 2 con
    Serial.println("ARDUINO READY");
    Serial.println("Nhap:");
    Serial.println("1 = bat LED ben ESP32");
    Serial.println("0 = tat LED ben ESP32");
}

void loop() {

    // =================================
    // ESP32 -> Arduino
    // =================================
    if (ESPSerial.available()) {

        char cmd = ESPSerial.read();

        if (cmd == '1') {

            digitalWrite(LED_ARDUINO, HIGH);

            Serial.println("ESP32 yeu cau:");
            Serial.println("LED Arduino ON");
        }

        else if (cmd == '0') {

            digitalWrite(LED_ARDUINO, LOW);

            Serial.println("ESP32 yeu cau:");
            Serial.println("LED Arduino OFF");
        }
    }

    // =================================1
    // PC -> Arduino -> ESP32
    // =================================
    if (Serial.available()) {

        char cmd = Serial.read();

        if (cmd == '1' || cmd == '0') {

            ESPSerial.write(cmd);

            Serial.print("Arduino gui ESP32: ");
            Serial.println(cmd);
        }
    }
}