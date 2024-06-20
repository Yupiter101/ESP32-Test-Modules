
#include "hmc.h"


Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345); // =-=-=HMC5883=-=-=-=-=-


bool initHMC (void) {
  /* === Initialise HMC5883 sensor == */
    if(!mag.begin()) {
        Serial.println("Ooops, HMC5883 sensor is FAILED!!!");
        return false;
    }
    else {
        Serial.println("HMC5883 inited");
        // show_HMC5883_details();
        return true;
    }
}


void show_HMC5883_details(void) {
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
//   delay(1000);
}


void show_HMC5883_values(void) {
    sensors_event_t event; 
    mag.getEvent(&event);
        
    /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
    // Serial.print("HMC5883 ");
    Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.print("uT  ");

    // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
    // Calculate heading when the magnetometer is level, then correct for signs of axis.
    float heading = atan2(event.magnetic.y, event.magnetic.x);
        
    // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
    // Find yours here: http://www.magnetic-declination.com/
    // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
    // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
    float declinationAngle = 0.22;
    heading += declinationAngle;
        
    // Correct for when signs are reversed.
    if(heading < 0)
        heading += 2*PI;
            
    // Check for wrap due to addition of declination.
    if(heading > 2*PI)
        heading -= 2*PI;
        
    // Convert radians to degrees for readability.
    float headingDegrees = heading * 180/M_PI; 
        
    Serial.print("Degr: "); Serial.println(headingDegrees);  
    // Serial.println();  
}


bool check_HMC_values (void) {
    sensors_event_t event; 
    mag.getEvent(&event);

    bool X_flag = false; //  
    bool Y_flag = false;
    bool Z_flag = false;
    float X_firstVal = event.magnetic.x;
    float Y_firstVal = event.magnetic.y;
    float Z_firstVal = event.magnetic.z;

    Serial.print("First Val: ");
    Serial.print(X_firstVal);
    Serial.print("  ");
    Serial.print(Y_firstVal);
    Serial.print("  ");
    Serial.println(Z_firstVal);
    delay(20); // Пауза між зчитуванням
    for(int i=0; i<20; i++) {
        sensors_event_t event; 
        mag.getEvent(&event);
        float X_restVal = event.magnetic.x;
        float Y_restVal = event.magnetic.y;
        float Z_restVal = event.magnetic.z;

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


