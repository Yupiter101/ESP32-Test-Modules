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

// Magnetometer-compas
QMC5883LCompass compass;  
// MPU9250 - accelerom, giro
#define MPU9250_ADDR 0x68
MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

// == Change I2C pins ==
#define I2C_SDA 33
#define I2C_SCL 32

#define LED_RED 15
#define LED_GREEN 25

// Тут готувати роботу з кнопкою
const uint8_t BUTTON_PIN = 35;  // the number of the pushbutton pin
bool buttonState = false;
uint32_t lastMillis = 0;
bool flagStartCheckModules = false;
int countBlink = 0;
uint16_t blinkInterval = 1000;
bool ledState = false;

// --------------------------

// boolean flag_BME_Init = false;
// boolean flag_HMC_Init = false;

int i2cAdrArr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // масив I2C адресів
uint8_t modulesStatus = 0b00000000; // Наявні I2C модулі. макс 8

#define QMC_Status modulesStatus & (1<<0)
#define HMC_Status modulesStatus & (1<<1)
#define OLED_Status modulesStatus & (1<<2)
#define MPU_Status modulesStatus & (1<<3)
#define BME_Status modulesStatus & (1<<4)




// Прототипи функцій. Винести в окремий файл
void scanningModules (void);
void innitMPU9250(void);
void buzzingGoog (void);
void buzzingBed (void);
bool check_QMC_values (void);
bool check_MPU_ACC_values (void);
bool check_MPU_Giro_values (void);
bool check_MPU_Temp_values (void);






void setup() {

    /* == SERIAL == */
    Serial.begin(115200);
    while(!Serial);    // time to get serial running

    /* == WIRE == */
    Wire.begin(I2C_SDA, I2C_SCL); // 33 32

    /* == PINS IN-OUT == */
    pinMode(LED_BUILTIN, OUTPUT); // Buzzer pin (2)
    pinMode(LED_RED, OUTPUT); // 
    pinMode(LED_GREEN, OUTPUT); // 
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Btn pin HIGH, wait LOW

}


void loop() { 

    // === блінк - режим очікування (нічого не робить) ===
    if(!flagStartCheckModules && millis() - lastMillis >= blinkInterval){
      lastMillis = millis();
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
      blinkInterval = ledState ? 30 : 970;
    } 

    // === Чи натиснута кнопка ===
    if(!buttonState && !digitalRead(BUTTON_PIN)) { 
      delay(5);  // Затримка від дребезга
      if(!digitalRead(BUTTON_PIN)) { // Перевірка натискання
        buttonState = true;        // Підняли Флаг натиснутої кнопки
        digitalWrite(LED_BUILTIN, LOW);
        ledState = false;
        delay(100); // Затримка від подвійного натискання
        blinkInterval = 1000;
                   
        flagStartCheckModules = true;   // Підняли Флаг про дозвіл перевірки модулів
      }
    }




    // === Якщо кнопку натиснули то почати перевірку показників модулів 
    if(flagStartCheckModules) { // Якщо є дозвіл перевірки модулів
      flagStartCheckModules = false;
      bool flag_BME_Init = false;
      bool flag_HMC_Init = false;

      /* === I2C scanner == */
      scanningModules();



      /* === INIT QMC5883 sensor magnetometr == */
      if(QMC_Status) {  // Якщо Є такий I2C адрес то почати ініціалізацію
        compass.init(); // Тут просто ініціалізація
        Serial.println("QMC5883 checking values...");

        bool flag_QMC_checked = check_QMC_values(); 
        if(flag_QMC_checked) {
            // good
            Serial.println("QMC5883 values GOOD");
            buzzingGoog();
        }
        else {
            // bed
            Serial.println("QMC5883 values BED");
            buzzingBed();
        }
        Serial.println(" ");
      }

      delay(1000);
        


      /* === INIT HMC5883 sensor magnetometr == */
      if(HMC_Status) {  // Якщо Є такий I2C адрес то почати ініціалізацію
        flag_HMC_Init = initHMC(); // Тут Вдалося ініціалізувати чи ні
        if(flag_HMC_Init) {  // Якщо вдалося ініціалізувати то почати зчитувати значення
          Serial.println("HMC5883 checking values...");
          bool flag_HMC_checked = check_HMC_values();

          if(flag_HMC_checked) {
            // good
            Serial.println("HMC5883 values GOOD");
            buzzingGoog();
          }
          else {
            // bed
            Serial.println("HMC5883 values BED");
            buzzingBed();
          }
        }
        else {
          // bed
          Serial.println("HMC5883 values BED");
          buzzingBed();
        }
        Serial.println(" ");
      }

      delay(1000);



      /* === INIT MPU9250 sensor Accereration Giro Mag Temp === */
      if(MPU_Status) {   // Якщо Є такий I2C адрес то почати ініціалізацію
        innitMPU9250(); // Тут просто ініціалізація
        Serial.println(" ");
        Serial.println("MPU9250 check values:");

        bool flag_Acc = check_MPU_ACC_values(); 
        bool flag_Giro = check_MPU_Giro_values(); 
        bool flag_Temp = check_MPU_Temp_values(); 

        if(flag_Acc && flag_Giro && flag_Temp) {
            // good
            Serial.println("MPU9250 values GOOD");
            buzzingGoog();
        }
        else {
            // bed
            Serial.println("MPU9250 values BED");
            buzzingBed();
        }
        Serial.println(" ");
      }
      delay(1000);



      /* === INIT BME280 sensor barometr === */
      if(BME_Status) {  // Якщо Є такий I2C адрес то почати ініціалізацію
        flag_BME_Init = initBME280(); // Тут Вдалося ініціалізувати чи ні
        if(flag_BME_Init) {  // Якщо вдалося ініціалізувати то почати зчитувати значення
          Serial.println("BME280 checking values...");
          bool flag_BME_checked = check_BME_values();

          if(flag_BME_checked) {
            // good
            Serial.println("BME280 values GOOD");
            buzzingGoog();
          }
          else {
            // bed
            Serial.println("BME280 values BED");
            buzzingBed();
          }
        }
        else {
          // bed
          Serial.println("BME280 values BED");
          buzzingBed();
        }
        Serial.println(" ");
      }
      delay(1000);
    }


  if(buttonState) {
    buttonState = false;
    modulesStatus = 0b00000000; // Обнуляем дані про скановані модулі
  }

}







// === FUNCTIONS ===

/* === I2C scanner == */
void scanningModules (void) {
  byte error, address; //variable for error and I2C address
    int nDevices = 0;
    Serial.println("\nScanning...");

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

    // while(!nDevices){
    //   digitalWrite(LED_BUILTIN, HIGH);
    //   delay(50); 
    //   digitalWrite(LED_BUILTIN, LOW);
    //   delay(450);
    // }
}

/* === Initialise MPU9250 sensor All data === */
void innitMPU9250(void) {
  if(!myMPU9250.init()){
        Serial.println("MPU9250 does not respond");
      }
      else{
        Serial.println("MPU9250 is connected");
      }
      if(!myMPU9250.initMagnetometer()){
        Serial.println("Magnetometer does not respond");
      }
      else{
        Serial.println("Magnetometer is connected");
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

      /*  The gyroscope data is not zero, even if you don't move the MPU9250. 
      *  To start at zero, you can apply offset values. These are the gyroscope raw values you obtain
      *  using the +/- 250 degrees/s range. 
      *  Use either autoOffset or setGyrOffsets, not both.
      */
      //myMPU9250.setGyrOffsets(45.0, 145.0, -105.0);

      /*  You can enable or disable the digital low pass filter (DLPF). If you disable the DLPF, you 
      *  need to select the bandwdith, which can be either 8800 or 3600 Hz. 8800 Hz has a shorter delay,
      *  but higher noise level. If DLPF is disabled, the output rate is 32 kHz.
      *  MPU9250_BW_WO_DLPF_3600 
      *  MPU9250_BW_WO_DLPF_8800
      */
      myMPU9250.enableGyrDLPF();
      //myMPU9250.disableGyrDLPF(MPU9250_BW_WO_DLPF_8800); // bandwdith without DLPF
      
      /*  Digital Low Pass Filter for the gyroscope must be enabled to choose the level. 
      *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7 
      *  
      *  DLPF    Bandwidth [Hz]   Delay [ms]   Output Rate [kHz]
      *    0         250            0.97             8
      *    1         184            2.9              1
      *    2          92            3.9              1
      *    3          41            5.9              1
      *    4          20            9.9              1
      *    5          10           17.85             1
      *    6           5           33.48             1
      *    7        3600            0.17             8
      *    
      *    You achieve lowest noise using level 6  
      */
      myMPU9250.setGyrDLPF(MPU9250_DLPF_6);

      /*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
      *  Sample rate = Internal sample rate / (1 + divider) 
      *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
      *  Divider is a number 0...255
      */
      myMPU9250.setSampleRateDivider(5);

      /*  MPU9250_GYRO_RANGE_250       250 degrees per second (default)
      *  MPU9250_GYRO_RANGE_500       500 degrees per second
      *  MPU9250_GYRO_RANGE_1000     1000 degrees per second
      *  MPU9250_GYRO_RANGE_2000     2000 degrees per second
      */
      myMPU9250.setGyrRange(MPU9250_GYRO_RANGE_250);

      /*  MPU9250_ACC_RANGE_2G      2 g   (default)
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
      //myMPU9250.enableGyrAxes(MPU9250_ENABLE_XYZ);
      
      /*
      * AK8963_PWR_DOWN       
      * AK8963_CONT_MODE_8HZ         default
      * AK8963_CONT_MODE_100HZ
      * AK8963_FUSE_ROM_ACC_MODE 
      */
      myMPU9250.setMagOpMode(AK8963_CONT_MODE_100HZ);
      delay(200);
}


// Buzzer sound module is good 
void buzzingGoog (void) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
    digitalWrite(LED_GREEN, LOW);
}
   
// Buzzer sound module is bed  
void buzzingBed (void) {
    digitalWrite(LED_RED, HIGH);
    for(int i=0; i<3; i++){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(150);
        digitalWrite(LED_BUILTIN, LOW);
        delay(150);
    }
    delay(2000);
    digitalWrite(LED_RED, LOW);
}



bool check_QMC_values (void) {
 
  bool X_flag = false; //  
  bool Y_flag = false;
  bool Z_flag = false;

  compass.read();
  int X_firstVal = compass.getX();
  int Y_firstVal = compass.getY();
  int Z_firstVal = compass.getZ();

  Serial.print("First Val: ");
  Serial.print(X_firstVal);
  Serial.print("  ");
  Serial.print(Y_firstVal);
  Serial.print("  ");
  Serial.println(Z_firstVal);
  delay(20); // Пауза між зчитуванням

  for(int i=0; i<20; i++) {
    compass.read();
    int X_restVal = compass.getX();
    int Y_restVal = compass.getY();
    int Z_restVal = compass.getZ();

    Serial.print("Rest Val: ");
    Serial.print(X_restVal);
    Serial.print("  ");
    Serial.print(Y_restVal);
    Serial.print("  ");
    Serial.println(Z_restVal);

    if(!X_flag && X_firstVal != X_restVal) {
      X_flag = true;
      Serial.println("X - Ok");
    }

    if(!Y_flag && Y_firstVal != Y_restVal) {
      Y_flag = true;
      Serial.println("Y - Ok");
    }

    if(!Z_flag && Z_firstVal != Z_restVal) {
      Z_flag = true;
      Serial.println("Z - Ok");
    }

    if(X_flag && Y_flag && Z_flag) {
      return true;
    }
    delay(200);
  }
  return false;
}




bool check_MPU_ACC_values (void) {
  Serial.println("Check Acceleration vall...");
  bool X_flag = false; //  
  bool Y_flag = false;
  bool Z_flag = false;

  xyzFloat gValue = myMPU9250.getGValues();
  
  float X_firstVal = gValue.x;
  float Y_firstVal = gValue.y;
  float Z_firstVal = gValue.z;
  Serial.print("First Val: ");
  Serial.print(X_firstVal, 4);
  Serial.print("  ");
  Serial.print(Y_firstVal, 4);
  Serial.print("  ");
  Serial.println(Z_firstVal, 4);

  delay(20); // Пауза між зчитуванням

  for(int i=0; i<20; i++) {
    xyzFloat gValue = myMPU9250.getGValues();

    float X_restVal = gValue.x;
    float Y_restVal = gValue.y;
    float Z_restVal = gValue.z;

    Serial.print("Rest Val: ");
    Serial.print(X_restVal, 4);
    Serial.print("  ");
    Serial.print(Y_restVal, 4);
    Serial.print("  ");
    Serial.println(Z_restVal, 4);

    if(!X_flag && X_firstVal != X_restVal) {
      X_flag = true;
      Serial.println("X - Ok");
    }

    if(!Y_flag && Y_firstVal != Y_restVal) {
      Y_flag = true;
      Serial.println("Y - Ok");
    }

    if(!Z_flag && Z_firstVal != Z_restVal) {
      Z_flag = true;
      Serial.println("Z - Ok");
    }

    if(X_flag && Y_flag && Z_flag) {
      return true;
      // flagNext = true;
    }
    delay(200);
  }
  return false;
}
  


bool check_MPU_Giro_values (void) {

  Serial.println("Check Giroscope vall...");
  bool X_flag = false; //  
  bool Y_flag = false;
  bool Z_flag = false;

  xyzFloat gyr = myMPU9250.getGyrValues();
  
  float X_firstVal = gyr.x;
  float Y_firstVal = gyr.y;
  float Z_firstVal = gyr.z;
  Serial.print("First Val: ");
  Serial.print(X_firstVal);
  Serial.print("  ");
  Serial.print(Y_firstVal);
  Serial.print("  ");
  Serial.println(Z_firstVal);

  delay(20); // Пауза між зчитуванням

  for(int i=0; i<20; i++) {
    xyzFloat gyr = myMPU9250.getGyrValues();

    float X_restVal = gyr.x;
    float Y_restVal = gyr.y;
    float Z_restVal = gyr.z;

    Serial.print("Rest Val: ");
    Serial.print(X_restVal);
    Serial.print("  ");
    Serial.print(Y_restVal);
    Serial.print("  ");
    Serial.println(Z_restVal);

    if(!X_flag && X_firstVal != X_restVal) {
      X_flag = true;
      Serial.println("X - Ok");
    }

    if(!Y_flag && Y_firstVal != Y_restVal) {
      Y_flag = true;
      Serial.println("Y - Ok");
    }

    if(!Z_flag && Z_firstVal != Z_restVal) {
      Z_flag = true;
      Serial.println("Z - Ok");
    }

    if(X_flag && Y_flag && Z_flag) {
      return true;
      // flagNext = true;
    }
    delay(200);
  }
  return false;
}


bool check_MPU_Temp_values (void) {
  Serial.println("Check Temperature vall..."); 
  float firstVal = myMPU9250.getTemperature();
  Serial.print("First Val: ");
  Serial.println(firstVal);

  delay(20); // Пауза між зчитуванням

  for(int i=0; i<20; i++) {
    float restVal = myMPU9250.getTemperature();
    Serial.print("Rest Val: ");
    Serial.println(restVal);

    if(firstVal != restVal) {
      return true;
    }
    delay(200);
  }
  return false;
}




// -------------------------------------

    // == SHOW_BME280 ==
    // if(flag_BME_Init) {  // Якщо вдалося ініціалізувати то почати зчитувати значення
    //     Serial.println("Log BME280");
    //     for(int i=0; i<10; i++) {
    //         show_BME280_values();
    //         delay(500);
    //     }
    //     delay(2000);
    // }
  
    // == SHOW_HMC5883 ==
    // if(flag_HMC_Init) {  // Якщо вдалося ініціалізувати то почати зчитувати значення
    //     Serial.println("Log HMC5883");
    //     for(int i=0; i<10; i++) {
    //         show_HMC5883_values();
    //         delay(500);
    //     }
    //     delay(2000);
    // }


    // // == SHOW_QMC5883 ==
    // if(QMC_Status) { // Тут почати зчитувати значення без попередньої перевірки на ініціалізацію
    //   Serial.println("Log QMC5883"); 

    //   for(int i=0; i<10; i++) {
    //     int x, y, z;  
    //     // Read compass values
    //     compass.read();

    //     // Return XYZ readings
    //     x = compass.getX();
    //     y = compass.getY();
    //     z = compass.getZ();
        
    //     Serial.print("X: ");
    //     Serial.print(x);
    //     Serial.print(" Y: ");
    //     Serial.print(y);
    //     Serial.print(" Z: ");
    //     Serial.println(z);
    //     // Serial.println(); 

    //     delay(500); 
    //   }
    //   Serial.println(); 
    // }

    // // == SHOW_MPU9250 Acc+Giro+Temp ==
    // if(MPU_Status) {
    //   Serial.println("Log MPU9250");

    //   for(int i=0; i<10; i++) {
    //     xyzFloat gValue = myMPU9250.getGValues();
    //     xyzFloat gyr = myMPU9250.getGyrValues();
    //     xyzFloat magValue = myMPU9250.getMagValues();
    //     float temp = myMPU9250.getTemperature();
    //     float resultantG = myMPU9250.getResultantG(gValue);

    //     Serial.print("Acceleration g (x,y,z): ");
    //     Serial.print(gValue.x);
    //     Serial.print("   ");
    //     Serial.print(gValue.y);
    //     Serial.print("   ");
    //     Serial.print(gValue.z);
    //     Serial.print("   ");
    //     Serial.print("Result g: ");
    //     Serial.println(resultantG);

    //     Serial.print("Gyroscope degrees/s: ");
    //     Serial.print(gyr.x);
    //     Serial.print("   ");
    //     Serial.print(gyr.y);
    //     Serial.print("   ");
    //     Serial.println(gyr.z);

    //     Serial.print("Magnetometer µTesla: ");
    //     Serial.print(magValue.x);
    //     Serial.print("   ");
    //     Serial.print(magValue.y);
    //     Serial.print("   ");
    //     Serial.println(magValue.z);

    //     Serial.print("Temperature in °C: ");
    //     Serial.println(temp);
    //     Serial.println();
    //     delay(1000);
    //   }
    //   delay(2000); 
    // }