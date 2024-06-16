/*
    Завдання:
    - Додати кнопку з Interrupt перемикання між модулями
    - Замінити delay() на millis()
        + Винести кожний модуль в окремий файл
        + Розбити окремі файли на .h та .cpp
    - Слідкувати за змінами величин від модулів. Якщо ні то помилка
    - Додати звукові сигнали (що усе ок)
        + Додати модуль BME280
        + Додати модуль HMC5883
    - Додати Сканер I2C
    - Додати акселерометр MPU-9250
    - Додати сімкарту
    - Додати роботу з GPS
    - Реалізувати спілкування між двома ESP32 
        + Залить на GIT
    - Device at address HMC5886 MPU9250  BME280
    - Device at address  0x1E    0x68     0x76
    - Device at analog   ----    0x69     ----
*/


#include <Arduino.h>
// #include <Wire.h>
#include "bme280.h"
#include "hmc.h"


#define I2C_SDA 33
#define I2C_SCL 32

boolean flag_BME_Init = true;
boolean flag_HMC_Init = true;
// uint32_t lastMilisBME = 0;
int i2cArrAdr[5]; // масив адресів


void setup() {
    delay(1);
    Serial.begin(115200);
    while(!Serial);    // time to get serial running
    Wire.begin(I2C_SDA, I2C_SCL); // 33 32



    /* === I2C scanner == */
    byte error, address; //variable for error and I2C address
    int nDevices = 0;
    

    Serial.println("Scanning...");

    // nDevices = 0;
    for (address = 1; address < 127; address++ )
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();

      if (error == 0)
      {
        Serial.print("Device at address 0x");
        if (address < 16)  Serial.print("0");
        Serial.println(address, HEX);
        i2cArrAdr[nDevices] = address;
        nDevices++;
      }
      else if (error == 4)
      {
        Serial.print("Error at address 0x");
        if (address < 16) Serial.print("0");
        Serial.println(address, HEX);
      }
    }
    if (nDevices == 0)
      Serial.println("No I2C devices found\n");
    else
      Serial.println("done\n");

    // while(1){
    //   delay(5); // wait 5 seconds for the next I2C scan
    // }



  

    /* === Initialise HMC5883 sensor == */
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
            delay(500);
        } 
    }
    delay(2000);

    // if(millis() - lastMilisBME >= 1000){
    //   lastMilisBME = millis();
    // }

    Serial.println(); 

    // == show_HMC5883 ==
    if(flag_HMC_Init) {
        Serial.println("Log HMC5883");
        for(int i=0; i<10; i++) {
            show_HMC5883_values();
            delay(500);
        } 
    }

    delay(2000);
    Serial.println();
}



