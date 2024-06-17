# ESP32-Test-Modules
Hire test BME380, HMC5883, MPU9250, SD-Card

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


    Инфа про MPU
    Є Приклад кода 
    - Pitch and roll
    - Giroscope
    - Who em I: 
        WhoAmI Register: 0x70 (I2C - 0x68)
        Your device is an MPU6500.
        The MPU6500 does not have a magnetometer.
    - Magnetometer does not respond


    Запитання:
    Як правильно полылити код на файли?
    Що повинно бути у .h що .cpp? (Бібліотеки змінні)
    Як підтягти бібліотеку у VS Код з GitHub