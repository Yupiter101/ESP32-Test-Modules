/*
    Завдання:
    - Додати кнопку з Interrupt перемикання між модулями
    - Замінити delay() на millis()
    - Винести кожний модуль в окремий файл
    - - Розбити окремі файли на .h та .cpp
    - Слідкувати за змінами величин від модулів. Якщо ні то помилка
    - Додати звукові сигнали (що усе ок)
    + Додати модуль BME280
    + Додати модуль HMC5883
    - Додати Сканер I2C
    - Додати акселерометр MPU-9250
    - Додати сімкарту
    - Додати роботу з GPS
    - Реалізувати спілкування між двома ESP32 
    - Залить на GIT
*/


#include <Arduino.h>
// #include <Wire.h>
#include "bme280.h"
#include "hmc.h"


#define I2C_SDA 33
#define I2C_SCL 32

boolean flag_BME_Init = true;
boolean flag_HMC_Init = true;



void setup() {
    delay(1);
    Serial.begin(115200);
    while(!Serial);    // time to get serial running
    Wire.begin(I2C_SDA, I2C_SCL); // 33 32


    // /* === Initialise HMC5883 sensor == */
    flag_HMC_Init = initHMC();
  
    delay(500);

    /* === Initialise BME280 sensor === */
    flag_BME_Init = initBME280();

}


void loop() { 
    // == show_BME280 ==
    if(flag_BME_Init) {
        Serial.println("Log BME280");
        for(int i=0; i<10; i++) {
            show_BME280_values();
            delay(1000);
        } 
    }
    delay(2000);
    Serial.println(); 

    // == show_HMC5883 ==
    if(flag_HMC_Init) {
        Serial.println("Log HMC5883");
        for(int i=0; i<10; i++) {
            show_HMC5883_values();
            delay(1000);
        } 
    }

    delay(2000);
    Serial.println();
}



