
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define SO_LED 1

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

Adafruit_NeoPixel led(SO_LED, LED_PIN, NEO_GRB + NEO_KHZ800);

class XuLyLenh : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String lenh = pCharacteristic->getValue().c_str();
        lenh.trim();
        lenh.toUpperCase();

        Serial.print("Nhan lenh: ");
        Serial.println(lenh);

        if (lenh == "1" || lenh == "G")
        {
            led.setPixelColor(0, led.Color(0, 255, 0));
        }
        else if (lenh == "0")
        {
            led.setPixelColor(0, led.Color(0, 0, 0));
        }
        else if (lenh == "R")
        {
            led.setPixelColor(0, led.Color(255, 0, 0));
        }
        else if (lenh == "B")
        {
            led.setPixelColor(0, led.Color(0, 0, 255));
        }
        else if (lenh == "W")
        {
            led.setPixelColor(0, led.Color(255, 255, 255));
        }

        led.show();
    }
};

void setup()
{
    Serial.begin(115200);

    led.begin();
    led.setBrightness(40);
    led.clear();
    led.show();

    BLEDevice::init("ESP32S3_TUAN");

    BLEServer *server = BLEDevice::createServer();
    BLEService *service = server->createService(SERVICE_UUID);

    BLECharacteristic *characteristic =
        service->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE
        );

    characteristic->setCallbacks(new XuLyLenh());
    characteristic->setValue("0");

    service->start();

    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);

    BLEDevice::startAdvertising();

    Serial.println("BLE SAN SANG");
    Serial.println("Tim thiet bi ESP32S3_TUAN tren iPhone");
}

void loop()
{
    delay(100);
}
