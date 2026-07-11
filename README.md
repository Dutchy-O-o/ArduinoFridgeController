# ArduinoFridgeController

An Arduino-based **smart refrigerator controller** written in embedded C (a 3-person university project).

## Features
- Reads a **DHT11** temperature/humidity sensor
- Drives a **DC motor** (compressor) with **PWM** speed and direction control via an H-bridge
- **HC-SR04** ultrasonic sensor for distance/level detection
- Live status on a **16x2 I2C LCD**
- Buzzer and LED alerts, with push-button control
- Non-blocking timing using `millis()` for responsive real-time behaviour

## Hardware
| Component | Pin / Interface |
|---|---|
| DHT11 temperature / humidity | A1 |
| I2C LCD (LiquidCrystal_I2C) | I2C (SDA/SCL) |
| DC motor (H-bridge) | PWM (pin 8), IN1/IN2 (pins 10/9) |
| HC-SR04 ultrasonic | TRIG 5, ECHO 4 |
| Buzzer | pin 7 |
| Push button | pin 2 |
| LED | pin 13 |

## Build
Open `smart_refrigerator.ino` in the Arduino IDE, install the **DHT** and **LiquidCrystal_I2C** libraries, select your board, and upload.
