#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX711.h>

#define VALVE_RELAY_PIN 26
#define PUMP_RELAY_PIN 25

#define TEMP_BUS 33
#define SCALE_DOUT_PIN 32
#define SCALE_SCK_PIN 27

double HX711_SCALING_FACTOR = 645.103969754;

// Setup a oneWire instance to communicate with any Temp device
OneWire oneWire(TEMP_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

HX711 scale;

// Define the service and characteristic UUIDs
BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
BLEUUID tempCharacteristicUUID("93053d0c-8729-11ee-b9d1-0242ac120002");
BLEUUID pumpCharacteristicUUID("a01911b6-872a-11ee-b9d1-0242ac120002");
BLEUUID valveCharacteristicUUID("5716efc6-cd9b-4525-ac8e-e54c1f6e3141");
BLEUUID weightCharacteristicUUID("9532b459-dc13-4651-9a56-ef6d1bfc2c0b");

// Create a BLE server
BLEServer *pServer = NULL;
BLECharacteristic *pTempCharacteristic = NULL;
BLECharacteristic *pPumpCharacteristic = NULL;
BLECharacteristic *pValveCharacteristic = NULL;
BLECharacteristic *pWeightCharacteristic = NULL;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    Serial.println("Client connected");
  };

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("Client disconnected");
    pServer->getAdvertising()->start();
  }
};

class PumpCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
      Serial.print("Received over BLE: ");
      Serial.println(value.c_str());

      if (value[0] == '1')
      {
        digitalWrite(PUMP_RELAY_PIN, HIGH);
      }
      else if (value[0] == '0')
      {
        digitalWrite(PUMP_RELAY_PIN, LOW);
      }
    }
  }
};

class ValveCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
      Serial.print("Received over BLE: ");
      Serial.println(value.c_str());

      if (value[0] == '1')
      {
        digitalWrite(VALVE_RELAY_PIN, HIGH);
      }
      else if (value[0] == '0')
      {
        digitalWrite(VALVE_RELAY_PIN, LOW);
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);

  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(VALVE_RELAY_PIN, OUTPUT);

  // Initialize the scale
  scale.begin(SCALE_DOUT_PIN, SCALE_SCK_PIN);
  scale.set_scale(HX711_SCALING_FACTOR);
  if (scale.wait_ready_timeout(500))
  {
    scale.tare();
  }
  else
  {
    // TODO: handle this
    Serial.println("Scale not connected");
  }

  // Create the BLE device
  BLEDevice::init("ESP32_SMART_LIVEWELL");

  // Create the BLE server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE service
  BLEService *pService = pServer->createService(serviceUUID);

  // Create control characteristics
  pTempCharacteristic = pService->createCharacteristic(
      tempCharacteristicUUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);

  pPumpCharacteristic = pService->createCharacteristic(
      pumpCharacteristicUUID,
      BLECharacteristic::PROPERTY_WRITE);
  pPumpCharacteristic->setCallbacks(new PumpCharacteristicCallbacks());

  pValveCharacteristic = pService->createCharacteristic(
      valveCharacteristicUUID,
      BLECharacteristic::PROPERTY_WRITE);
  pValveCharacteristic->setCallbacks(new ValveCharacteristicCallbacks());

  pWeightCharacteristic = pService->createCharacteristic(
      weightCharacteristicUUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();

  Serial.println("BLE server started");
}

void loop()
{
  sensors.requestTemperatures();                  // Send the command to get temperatures
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

  if (scale.wait_ready_timeout(1000))
  {
    // Get scale reading
    float weight = scale.get_units(10);

    // Convert weight to a string
    char weightString[20];
    sprintf(weightString, "%f", weight);

    Serial.print("Weight: ");
    Serial.print(weightString);
    Serial.println(" oz");

    pWeightCharacteristic->setValue(weightString);
    pWeightCharacteristic->notify();
  }
  else
  {
    Serial.println("scale not found");
  }

  delay(1000);
}
