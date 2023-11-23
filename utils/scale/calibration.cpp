#include <HX711.h>

#define SCALE_DOUT_PIN 32
#define SCALE_SCK_PIN 27

HX711 scale;

void setup()
{
    Serial.begin(115200);
    scale.begin(SCALE_DOUT_PIN, SCALE_SCK_PIN);
}

void loop()
{
    if (scale.is_ready())
    {
        scale.set_scale();
        Serial.println("Tare... remove any weights from the scale.");
        delay(5000);
        scale.tare();
        Serial.println("Tare done...");
        Serial.print("Place a known weight on the scale...");
        delay(5000);
        long reading = scale.get_units(10);
        Serial.print("Result: ");
        Serial.println(reading);
    }
    else
    {
        Serial.println("HX711 not found.");
    }
    delay(1000);
}