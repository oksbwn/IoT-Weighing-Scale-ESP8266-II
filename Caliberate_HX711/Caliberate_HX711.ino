
#include "HX711.h"

//Calibration factor for HX711
#define CAL_FACTOR 11280.00 //This value is obtained using the caliberation sketch
#define CONV_FACTOR 0.453592

//Connecting GPIOs
#define DOUT 13
#define SCK 15

// Scale Object
HX711 myScale;


void setup() {
    myScale.begin(DOUT,SCK);
    myScale.set_scale(0.021900);
    myScale.tare();
    Serial.begin(115200);
    Serial.println("Place known Weight !");
    delay(10000);
}

void loop() {
    if (myScale.is_ready()) {
     Serial.println(myScale.get_units(10));
    }
     delay(10000);
}
