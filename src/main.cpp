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
// #include <Adafruit_Sensor.h>
#include "bme280.h"
#include "hmc.h"
// #include <Adafruit_HMC5883_U.h>

// Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345); // =-=-=HMC5883=-=-=-=-=-

#define I2C_SDA 33
#define I2C_SCL 32

boolean flag_BME_Init = true;
boolean flag_HMC_Init = true;

// void show_HMC5883_details(void);
// void show_HMC5883_values(void);



void setup() {
    delay(1);
    Serial.begin(115200);
    while(!Serial);    // time to get serial running
    Wire.begin(I2C_SDA, I2C_SCL);


    // /* === Initialise HMC5883 sensor == */
    // if(!mag.begin()) {
    //     Serial.println("Ooops, HMC5883 sensor is FAILED!!!");
    //     flag_HMC_Init = false;
    //     // while(1);
    // }
    // else {
    //     /* Display some basic information on this sensor */
    //     Serial.println("HMC5883 sensor OK!");
    //     // show_HMC5883_details();
    // }

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




// void show_HMC5883_values(void) {
//     sensors_event_t event; 
//     mag.getEvent(&event);
        
//     /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
//     // Serial.print("HMC5883 ");
//     Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
//     Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
//     Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.print("uT  ");

//     // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
//     // Calculate heading when the magnetometer is level, then correct for signs of axis.
//     float heading = atan2(event.magnetic.y, event.magnetic.x);
        
//     // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
//     // Find yours here: http://www.magnetic-declination.com/
//     // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
//     // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
//     float declinationAngle = 0.22;
//     heading += declinationAngle;
        
//     // Correct for when signs are reversed.
//     if(heading < 0)
//         heading += 2*PI;
            
//     // Check for wrap due to addition of declination.
//     if(heading > 2*PI)
//         heading -= 2*PI;
        
//     // Convert radians to degrees for readability.
//     float headingDegrees = heading * 180/M_PI; 
        
//     Serial.print("Degr: "); Serial.println(headingDegrees);  
//     // Serial.println();  
// }



// void show_HMC5883_details(void) {
//   sensor_t sensor;
//   mag.getSensor(&sensor);
//   Serial.println("------------------------------------");
//   Serial.print  ("Sensor:       "); Serial.println(sensor.name);
//   Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
//   Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
//   Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
//   Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
//   Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
//   Serial.println("------------------------------------");
//   Serial.println("");
// //   delay(1000);
// }