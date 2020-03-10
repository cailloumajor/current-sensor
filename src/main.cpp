#include <Arduino.h>
#include <Homie.h>

#define FIRMWARE_VERSION "1.0.0"

const int INTERVAL = 5; // Measurement interval in seconds

unsigned long lastMeasurement = 0;

HomieNode currentNode("current", "Current sensor", "current");
// clang-format off
HomieSetting<long> burdenResistorSetting(
    "burdenResistor", "The value of burden resistor in ohms"
);
// clang-format on

float getVpp()
{
    int adc;
    int minValue = 1024, maxValue = 0;

    unsigned long startMillis = millis();
    while ((unsigned long)(millis() - startMillis) < 1000UL) {
        adc = analogRead(A0);
        if (adc > maxValue) {
            maxValue = adc;
        }
        if (adc < minValue) {
            minValue = adc;
        }
    }

    return (maxValue - minValue) * 3.2 / 1023;
}

void loopHandler()
{
    unsigned long currentMillis = millis();
    if ((unsigned long)(currentMillis - lastMeasurement) >= (INTERVAL * 1000UL)
        || (lastMeasurement == 0 && Homie.isConnected())) {
        float current = getVpp() / 2 / sqrt(2) / burdenResistorSetting.get() * 2000;
        Homie.getLogger() << "Current: " << current << " A" << endl;
        currentNode.setProperty("amperes").send(String(current, 1));
        lastMeasurement = millis();
    }
}

void setup()
{
    Serial.begin(115200);
    Serial << endl << endl;

    Homie_setFirmware("current-sensor", FIRMWARE_VERSION);
    Homie.setLoopFunction(loopHandler);

    // clang-format off
    currentNode.advertise("amperes")
        .setName("Amperes")
        .setDatatype("float")
        .setUnit("A");
    burdenResistorSetting.setValidator([](long candidate) {
        return candidate > 0;
    });
    // clang-format on

    Homie.setup();
}

void loop()
{
    Homie.loop();
}
