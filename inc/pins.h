// pins.h
// Created on: 2025-09-23
// Author: Sebastien Cabana
// Description: ESP32 GPIO pin assignments for the JDG2026 Machine project.

#ifndef PINS_H
#define PINS_H

// Reserved GPIO pins on ESP32
#define RESERVED5_PIN       6       // Internal flash
#define RESERVED6_PIN       7       // Internal flash
#define RESERVED7_PIN       8       // Internal flash
#define RESERVED8_PIN       9       // Internal flash
#define RESERVED9_PIN       10      // Internal flash
#define RESERVED10_PIN      11      // Internal flash

// DC motor pins
#define MOTOR_A_IN1_PIN     32
#define MOTOR_A_IN2_PIN     25
#define MOTOR_A_EN_PIN      21
#define MOTOR_B_IN1_PIN     26
#define MOTOR_B_IN2_PIN     27
#define MOTOR_B_EN_PIN      19

// Servo pins
#define SERVO1_PIN          15
#define SERVO2_PIN          4

#endif // PINS_H