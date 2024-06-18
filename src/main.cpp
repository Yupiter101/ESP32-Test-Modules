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
    - Device at analog   0x0D    0x69     ----     0x3D
    - Device at dex     30(13)    104     118        60
*/


#include <Arduino.h>
// #include <Wire.h>
#include <QMC5883LCompass.h>
#include "bme280.h"
#include "hmc.h"


QMC5883LCompass compass;

#define I2C_SDA 33
#define I2C_SCL 32

boolean flag_BME_Init = false;
boolean flag_HMC_Init = false;
// uint32_t lastMilisBME = 0;
int i2cAdrArr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // масив I2C адресів
uint8_t modulesStatus = 0b00000000; // Наявні I2C модулі. макс 8


#define QMC_Status modulesStatus & (1<<0)
#define HMC_Status modulesStatus & (1<<1)
#define OLED_Status modulesStatus & (1<<2)
#define MPU_Status modulesStatus & (1<<3)
#define BME_Status modulesStatus & (1<<4)




void setup() {

    pinMode(LED_BUILTIN, OUTPUT); // Buzzer pin (2)
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
          
          case 0x0D:
          modulesStatus |= (1<<0); // Є QMC5886 значить 0 біт true
            Serial.println("QMC5886");
            break;
          case 0x1E:
          modulesStatus |= (1<<1); // Є HMC5886 значить 1 біт true
            Serial.println("HMC5886");
            break;
          case 0x3C:
          modulesStatus |= (1<<2); // Є OLED значить 2 біт true
            Serial.println("OLED-0.96");
            break;
          case 0x68:
            modulesStatus |= (1<<3); // Є MPU9250 значить 3 біт true
            Serial.println("MPU9250");
            break;
          case 0x76:
            modulesStatus |= (1<<4); // Є BME280 значить 4 біт true
            Serial.println("BME280");
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
      Serial.println("I2C done\n");

    while(!nDevices){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50); 
      digitalWrite(LED_BUILTIN, LOW);
      delay(450);
    }


    /* === Initialise HMC5883 sensor == */
    if(QMC_Status) {  // Якщо Є такий I2C адрес то почати ініціалізацію
      Serial.println("QMC5883 detected"); 
      // flag_HMC_Init = initHMC();
      compass.init(); // Тут просто ініціалізація
      
    }

    if(HMC_Status) {  // Якщо Є такий I2C адрес то почати ініціалізацію
      Serial.println("HMC5883 detected");
      flag_HMC_Init = initHMC(); // Тут Вдалося ініціалізувати чи ні
    }
  
    delay(500);

    /* === Initialise BME280 sensor === */
    if(BME_Status) {  // Якщо Є такий I2C адрес то почати ініціалізацію
      Serial.println("BME280 detected");
      flag_BME_Init = initBME280(); // Тут Вдалося ініціалізувати чи ні
    }
}


void loop() { 


    // == show_BME280 ==
    if(flag_BME_Init) {  // Якщо вдалося ініціалізувати то почати зчитувати значення
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
    if(flag_HMC_Init) {  // Якщо вдалося ініціалізувати то почати зчитувати значення
        Serial.println("Log HMC5883");
        for(int i=0; i<10; i++) {
            show_HMC5883_values();
            delay(500);
        } 
    }

    delay(2000);


    if(QMC_Status) { // Тут почати зчитувати значення без попередньої перевірки на ініціалізацію 
      int x, y, z;  
      // Read compass values
      compass.read();

      // Return XYZ readings
      x = compass.getX();
      y = compass.getY();
      z = compass.getZ();
      
      Serial.print("X: ");
      Serial.print(x);
      Serial.print(" Y: ");
      Serial.print(y);
      Serial.print(" Z: ");
      Serial.print(z);
      // Serial.println();
    }

    Serial.println();
}



