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
    - Device at address HMC5886 MPU9250  BME280  OLED-0.96
    - Device at address  0x1E    0x68     0x76     0x3C
    - Device at analog   ----    0x69     ----     0x3D
*/


#include <Arduino.h>
// #include <Wire.h>
#include "bme280.h"
#include "hmc.h"


#define I2C_SDA 33
#define I2C_SCL 32

boolean flag_BME_Init = false;
boolean flag_HMC_Init = false;
// uint32_t lastMilisBME = 0;
int i2cAdrArr[5] = { 0, 0, 0, 0, 0 }; // масив адресів
uint8_t modulesStatus = 0b00000000; // інформація про скановані модулі. макс 8

#define MPU_Status modulesStatus & (1<<0)
#define BME_Status modulesStatus & (1<<1)
#define HMC_Status modulesStatus & (1<<2)
#define OLED_Status modulesStatus & (1<<3)




void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    delay(1);
    Serial.begin(115200);
    while(!Serial);    // time to get serial running
    Wire.begin(I2C_SDA, I2C_SCL); // 33 32



    /* === I2C scanner == */
    byte error, address; //variable for error and I2C address
    int nDevices = 0;
    Serial.println("Scanning...");

    for (address = 1; address < 127; address++ )
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();

      if (error == 0)
      {
        Serial.print("Device at address 0x");
        if (address < 16)  Serial.print("0");
        Serial.print(address, HEX);
        
        i2cAdrArr[nDevices] = address;
        Serial.print(" - ");
        switch(address) {
          case 0x68:
            modulesStatus |= (1<<0); // Є MPU9250 значить 0 біт true
            Serial.println("MPU9250");
            break;
          case 0x76:
            modulesStatus |= (1<<1); // Є BME280 значить 1 біт true
            Serial.println("BME280");
            break;
          case 0x1E:
          modulesStatus |= (1<<2); // Є HMC5886 значить 2 біт true
            Serial.println("HMC5886");
            break;
          case 0x3C:
          modulesStatus |= (1<<3); // Є OLED значить 3 біт true
            Serial.println("OLED-0.96");
            break;
          default:
            Serial.println("No inform.");
            break;
        }
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

    while(!nDevices){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50); 
      digitalWrite(LED_BUILTIN, LOW);
      delay(450);
    }


    /* === Initialise HMC5883 sensor == */
    if(HMC_Status) {
      Serial.println("Enable HMC5883");
      flag_HMC_Init = initHMC();
    }
    else {
      Serial.println("Disable HMC5883");
    }
  
    delay(500);

    /* === Initialise BME280 sensor === */
    if(BME_Status) {
      Serial.println("Enable BME280");
      flag_BME_Init = initBME280();
    }
    else {
      Serial.println("Disable BME280");
    }

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



