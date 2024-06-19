#pragma once
#include <Adafruit_BME280.h>

// #pragma once
// #ifndef _BME280_H_
//   #define _BME280_H_
//   #include "bme280.h"
// #endif

// Працює BME280 adr = 0x76, default = 0x77


bool initBME280 (void);
void show_BME280_values(void);
bool check_BME_values (void);

