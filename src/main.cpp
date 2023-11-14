#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_BUS 33
#define RELAY_PIN 32

// Setup a oneWire instance to communicate with any Temp device
OneWire oneWire(TEMP_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Define the service and characteristic UUIDs
BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
BLEUUID tempCharacteristicUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
BLEUUID relayCharacteristicUUID("26e2b12b-85f0-4f3f-916f-5d0af7256323");

// Create a BLE server
BLEServer *pServer = NULL;
BLECharacteristic *pTempCharacteristic = NULL;
BLECharacteristic *pRelayCharacteristic = NULL;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Client connected");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Client disconnected");
      pServer->getAdvertising()->start();
    }
};


class RelayCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.print("Received over BLE: ");
        Serial.println(value.c_str());
        
        if (value[0] == '1') {
          digitalWrite(RELAY_PIN, HIGH);
        } else if (value[0] == '0') {
          digitalWrite(RELAY_PIN, LOW);
        }
      }
    }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE device
  BLEDevice::init("ESP32_SMART_LIVEWELL");

  // Create the BLE server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE service
  BLEService *pService = pServer->createService(serviceUUID);

  pTempCharacteristic = pService->createCharacteristic(
    tempCharacteristicUUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Relay control characteristic
  pRelayCharacteristic = pService->createCharacteristic(
    relayCharacteristicUUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRelayCharacteristic->setCallbacks(new RelayCharacteristicCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();

  Serial.println("BLE server started");
}

void loop() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  float temperature = sensors.getTempCByIndex(0); // Get temperature in Celsius
  
  // Convert temperature to a string
  char tempString[10];
  dtostrf(temperature, 1, 2, tempString);

  Serial.print("Temperature: ");
  Serial.print(tempString);
  Serial.println(" °C");

  // Send temperature over BLE
  pTempCharacteristic->setValue(tempString);
  pTempCharacteristic->notify();

  delay(1000); 
}
