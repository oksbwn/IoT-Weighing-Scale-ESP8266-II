
#include "HX711.h"

//Calibration factor for HX711
#define CAL_FACTOR 11280.00 //This value is obtained using the caliberation sketch
#define CONV_FACTOR 0.453592

//Connecting GPIOs
#define DOUT 13
#define CLK 15

HX711 scale(DOUT, CLK);

void setup() {
    scale.set_scale(25380);
    scale.tare();

    Serial.begin(115200);
    Serial.println();
}

void loop() {
     Serial.println(scale.get_units(10));
     delay(100);
}
