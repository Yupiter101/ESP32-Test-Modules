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
      + accelerometr
      - giro
      - magnetometr
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

#include <MPU9250_WE.h>

#include <QMC5883LCompass.h>
#include "bme280.h"
#include "hmc.h"

#define MPU9250_ADDR 0x68
QMC5883LCompass compass;
// MPU - accelerom, giro, magnetom
MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

#define I2C_SDA 33
#define I2C_SCL 32

// Тут готувати роботу з кнопкою
const uint8_t BUTTON_PIN = 35;  // the number of the pushbutton pin
bool buttonState = false;

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
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Btn pin HIGH, wait LOW

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
      Serial.println("No I2C devices!!!\n");
    else
      Serial.println("I2C done\n");

    while(!nDevices){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50); 
      digitalWrite(LED_BUILTIN, LOW);
      delay(450);
    }
    /* === End I2C scanner == */



    /* === Initialise HMC5883 sensor == */
    if(QMC_Status) {  // Якщо Є такий I2C адрес то почати ініціалізацію
      Serial.println("QMC5883 detected"); 
      // flag_HMC_Init = initHMC();
      compass.init(); // Тут просто ініціалізація
    }

    /* === Initialise QMC5883 sensor == */
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

    /* === Initialise MPU9250 sensor Accereration=== */
    if(MPU_Status) {   // Якщо Є такий I2C адрес то почати ініціалізацію

      if(!myMPU9250.init()){
        Serial.println("MPU9250 does not respond");
      }
      else{
        Serial.println("MPU9250 is connected");
      }


        /* The slope of the curve of acceleration vs measured values fits quite well to the theoretical 
      * values, e.g. 16384 units/g in the +/- 2g range. But the starting point, if you position the 
      * MPU9250 flat, is not necessarily 0g/0g/1g for x/y/z. The autoOffset function measures offset 
      * values. It assumes your MPU9250 is positioned flat with its x,y-plane. The more you deviate 
      * from this, the less accurate will be your results.
      * The function also measures the offset of the gyroscope data. The gyroscope offset does not   
      * depend on the positioning.
      * This function needs to be called at the beginning since it can overwrite your settings!
      */
      Serial.println("Position you MPU9250 flat and don't move it - calibrating...");
      delay(1000);
      myMPU9250.autoOffsets();
      Serial.println("Done!");
      
      /*  This is a more accurate method for calibration. You have to determine the minimum and maximum 
      *  raw acceleration values of the axes determined in the range +/- 2 g. 
      *  You call the function as follows: setAccOffsets(xMin,xMax,yMin,yMax,zMin,zMax);
      *  Use either autoOffset or setAccOffsets, not both.
      */
      //myMPU9250.setAccOffsets(-14240.0, 18220.0, -17280.0, 15590.0, -20930.0, 12080.0);

      /*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
      *  Sample rate = Internal sample rate / (1 + divider) 
      *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
      *  Divider is a number 0...255
      */
      myMPU9250.setSampleRateDivider(5);
      
      /*  MPU9250_ACC_RANGE_2G      2 g   
      *  MPU9250_ACC_RANGE_4G      4 g
      *  MPU9250_ACC_RANGE_8G      8 g   
      *  MPU9250_ACC_RANGE_16G    16 g
      */
      myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);

      /*  Enable/disable the digital low pass filter for the accelerometer 
      *  If disabled the bandwidth is 1.13 kHz, delay is 0.75 ms, output rate is 4 kHz
      */
      myMPU9250.enableAccDLPF(true);

      /*  Digital low pass filter (DLPF) for the accelerometer, if enabled 
      *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7 
      *   DLPF     Bandwidth [Hz]      Delay [ms]    Output rate [kHz]
      *     0           460               1.94           1
      *     1           184               5.80           1
      *     2            92               7.80           1
      *     3            41              11.80           1
      *     4            20              19.80           1
      *     5            10              35.70           1
      *     6             5              66.96           1
      *     7           460               1.94           1
      */
      myMPU9250.setAccDLPF(MPU9250_DLPF_6);

      /*  Set accelerometer output data rate in low power mode (cycle enabled)
      *   MPU9250_LP_ACC_ODR_0_24          0.24 Hz
      *   MPU9250_LP_ACC_ODR_0_49          0.49 Hz
      *   MPU9250_LP_ACC_ODR_0_98          0.98 Hz
      *   MPU9250_LP_ACC_ODR_1_95          1.95 Hz
      *   MPU9250_LP_ACC_ODR_3_91          3.91 Hz
      *   MPU9250_LP_ACC_ODR_7_81          7.81 Hz
      *   MPU9250_LP_ACC_ODR_15_63        15.63 Hz
      *   MPU9250_LP_ACC_ODR_31_25        31.25 Hz
      *   MPU9250_LP_ACC_ODR_62_5         62.5 Hz
      *   MPU9250_LP_ACC_ODR_125         125 Hz
      *   MPU9250_LP_ACC_ODR_250         250 Hz
      *   MPU9250_LP_ACC_ODR_500         500 Hz
      */
      //myMPU9250.setLowPowerAccDataRate(MPU9250_LP_ACC_ODR_500);

      /* sleep() sends the MPU9250 to sleep or wakes it up. 
      * Please note that the gyroscope needs 35 milliseconds to wake up.
      */
      //myMPU9250.sleep(true);

    /* If cycle is set, and standby or sleep are not set, the module will cycle between
      *  sleep and taking a sample at a rate determined by setLowPowerAccDataRate().
      */
      //myMPU9250.enableCycle(true);

      /* You can enable or disable the axes for gyroscope and/or accelerometer measurements.
      * By default all axes are enabled. Parameters are:  
      * MPU9250_ENABLE_XYZ  //all axes are enabled (default)
      * MPU9250_ENABLE_XY0  // X, Y enabled, Z disabled
      * MPU9250_ENABLE_X0Z   
      * MPU9250_ENABLE_X00
      * MPU9250_ENABLE_0YZ
      * MPU9250_ENABLE_0Y0
      * MPU9250_ENABLE_00Z
      * MPU9250_ENABLE_000  // all axes disabled
      */
      //myMPU9250.enableAccAxes(MPU9250_ENABLE_XYZ);

    }



    /* === Initialise MPU9250 sensor Giroscop === */
        // Дивитися з прикладу бібліотеки
    

}


void loop() { 

    // Тут готувати роботу з кнопкою
    buttonState = digitalRead(BUTTON_PIN);


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

    // == show_QMC5883 ==
    if(QMC_Status) { // Тут почати зчитувати значення без попередньої перевірки на ініціалізацію
      Serial.println("Log QMC5883"); 

      for(int i=0; i<10; i++) {
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
        Serial.println(z);
        // Serial.println(); 

        delay(500); 
      }
    Serial.println(); 
    delay(2000);

    }

    // == show_MPU9250 Accereration ==
    if(MPU_Status) {
      Serial.println("Log MPU9250");
      for(int i=0; i<10; i++) {
        xyzFloat accRaw = myMPU9250.getAccRawValues();
        xyzFloat accCorrRaw = myMPU9250.getCorrectedAccRawValues();
        xyzFloat gValue = myMPU9250.getGValues();
        float resultantG = myMPU9250.getResultantG(gValue);
        
        Serial.print("Raw acceleration values (x,y,z): ");
        Serial.print(accRaw.x);
        Serial.print("   ");
        Serial.print(accRaw.y);
        Serial.print("   ");
        Serial.println(accRaw.z);

        Serial.print("Acceleration values (x,y,z): ");
        Serial.print(accCorrRaw.x);
        Serial.print("   ");
        Serial.print(accCorrRaw.y);
        Serial.print("   ");
        Serial.println(accCorrRaw.z);

        Serial.print("g values (x,y,z): ");
        Serial.print(gValue.x);
        Serial.print("   ");
        Serial.print(gValue.y);
        Serial.print("   ");
        Serial.println(gValue.z);

        Serial.print("Resultant g: ");
        Serial.println(resultantG); // should always be 1 g if only gravity acts on the sensor.
        Serial.println();
        delay(1000);
      }
    delay(2000);
      
    }

    Serial.println();
}



