#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define LED 2

// Define the service and characteristic UUIDs
BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
BLEUUID characteristicUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

// Create a BLE server
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Client connected");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Client disconnected");
      pServer->startAdvertising();
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // Create the BLE device
  BLEDevice::init("ESP32_LED");

  // Create the BLE server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE service
  BLEService *pService = pServer->createService(serviceUUID);

  // Create the BLE characteristic
  pCharacteristic = pService->createCharacteristic(
    characteristicUUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE
  );

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();

  Serial.println("BLE server started");
}

void loop() {
  digitalWrite(LED, HIGH);
  Serial.println("LED is on");
  pCharacteristic->setValue("LED is on");
  pCharacteristic->notify();
  delay(1000);

  digitalWrite(LED, LOW);
  Serial.println("LED is off");
  pCharacteristic->setValue("LED is off");
  pCharacteristic->notify();
  delay(1000);
}
