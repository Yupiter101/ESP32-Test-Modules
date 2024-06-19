

#include "bme280.h"

#define I2C_ADR_BME 0x76
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI



bool initBME280 (void) {
    /* === Initialise BME280 sensor === */
    unsigned status;
    status = bme.begin(I2C_ADR_BME);  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Ooops, BME280 sensor is FAILED!!!");
        return false;
    }
    else {
        Serial.println("BME280 inited");
        return true;
    } 
}


void show_BME280_values(void) {
    // Serial.print("BME280  ");
    // Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.print(" °C  ");
    // Serial.print("Pres = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.print(" hPa  ");
    // Serial.print("Alt = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.print(" m  ");
    // Serial.print("Hum = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    // Serial.println();
}

bool check_BME_values (void) {
    float altitud_firstVal = bme.readAltitude(SEALEVELPRESSURE_HPA);
    for(int i=0; i<100; i++) {
        float altitud_restVal = bme.readAltitude(SEALEVELPRESSURE_HPA);
        if(altitud_firstVal != altitud_firstVal) {
           return true; 
        }
        delay(20);
    }     
    return false;
}


// #endif
