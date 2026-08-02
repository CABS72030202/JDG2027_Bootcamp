// pins.h
// Created on: 2025-09-23
// Author: Sebastien Cabana
// Description: ESP32 GPIO pin assignments for the JDG2026 Machine project.
//              Maps all peripheral connections including SPI, I2C, motors and sensors.

#ifndef PINS_H
#define PINS_H

// Reserved GPIO pins on ESP32
#define RESERVED5_PIN       6       // Internal flash
#define RESERVED6_PIN       7       // Internal flash
#define RESERVED7_PIN       8       // Internal flash
#define RESERVED8_PIN       9       // Internal flash
#define RESERVED9_PIN       10      // Internal flash
#define RESERVED10_PIN      11      // Internal flash

// VSPI pins
#define SPI_MISO_PIN        19      // Shared with STEPPER_A_MS1 and SERVO5
#define SPI_MOSI_PIN        23      // Shared with STEPPER_A_STEP
#define SPI_SCLK_PIN        18      // Shared with STEPPER_A_EN
#define SPI_SS_PIN          5       // Shared with STEPPER_A_DIR

// I2C pins
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22

// UART pins
#define UART_TX_PIN         1       // Pin TX
#define UART_RX_PIN         3       // Pin RX

// DC motor pins
#define MOTOR_A_IN1_PIN     25
#define MOTOR_A_IN2_PIN     26

#define MOTOR_B_IN1_PIN     27
#define MOTOR_B_IN2_PIN     32

// Servo pins
#define SERVO1_PIN          33
#define SERVO2_PIN          13
#define SERVO3_PIN          14
#define SERVO4_PIN          15
#define SERVO5_PIN          19      // Shared with STEPPER_A_MS1 and SPI_MISO
#define SERVO6_PIN          4       // Shared with STEPPER_B_MS1 and LED_4

// Stepper motor pins
#define STEPPER_A_STEP_PIN  23      // Shared with SPI_MOSI
#define STEPPER_A_DIR_PIN   5       // Shared with SPI_SS
#define STEPPER_A_EN_PIN    18      // Shared with SPI_SCK (active LOW)
#define STEPPER_A_MS1_PIN   19      // Shared with SPI_MISO

#define STEPPER_B_STEP_PIN  16      // Shared with LED_1
#define STEPPER_B_DIR_PIN   17      // Shared with LED_2
#define STEPPER_B_EN_PIN    12      // Shared with LED_3 (active LOW)
#define STEPPER_B_MS1_PIN   4       // Shared with LED_4

// LED pins 
#define WS2811_PIN          2
#define LED_1_PIN           16      // Shared with STEPPER_B_STEP (HIGH current output)
#define LED_2_PIN           17      // Shared with STEPPER_B_DIR (HIGH current output)
#define LED_3_PIN           12      // Shared with STEPPER_B_EN (HIGH current output)
#define LED_4_PIN           4       // Shared with STEPPER_B_MS1 and SERVO6 (HIGH current output)

// Input pins
#define INPUT_PIN_1         34      // Input only
#define INPUT_PIN_2         35      // Input only
#define INPUT_PIN_3         36      // Input only
#define INPUT_PIN_4         39      // Input only

#endif // PINS_H