// motor.cpp
// Created on: 2025-10-28
// Author: Sebastien Cabana
// Description: Implementation of motor control functions for various motor types.
//              Provides unified interface for DC, mecanum and stepper motor control.

#include <motor.h>

// Motor Correction Factors
int BASE_SPEED = 200;                // Default base analog speed for motors
int MINIMUM_SPEED = 150;             // Default minimum analog speed to overcome motor stall
float LEFT_CORRECTION = 1.0;         // Default correction factor for left motor
float RIGHT_CORRECTION = 1.0;        // Default correction factor for right motor

// Global Variables
int current_speed = 0;               // Current speed level index (0 to MAX_SPEED-1)
int current_direction = 0;           // Current direction code (0 to 8)
int left_speed = 0;                  // Current speed for the left motor (analog value)
int right_speed = 0;                 // Current speed for the right motor (analog value)

void setup_dc_motors(int base, int min, int left_correction, int right_correction) {
    // Setup motor correction factors
    BASE_SPEED = base;
    MINIMUM_SPEED = min;
    LEFT_CORRECTION = left_correction;
    RIGHT_CORRECTION = right_correction;

    // Initialize DC motor control pins
    pinMode(MOTOR_A_IN1_PIN, OUTPUT);
    pinMode(MOTOR_A_IN2_PIN, OUTPUT);
    pinMode(MOTOR_B_IN1_PIN, OUTPUT);
    pinMode(MOTOR_B_IN2_PIN, OUTPUT);
}

void set_dc_speed(int direction, int speed_level) {
    int speed_percent = SPEED_MULTIPLIERS[speed_level - 1];
    int base_speed = (BASE_SPEED * speed_percent) / 100;
    int left_base = base_speed * LEFT_CORRECTION;
    int right_base = base_speed * RIGHT_CORRECTION;

    if(MOTOR_DEBUG) {
        fprintf(stderr, "Direction: %d, Speed Level: %d | ", direction, speed_level);
        fprintf(stderr, "Base: %d, Left: %d, Right: %d\n", base_speed, left_base, right_base);
    }

    // Calculate speed for each motor based on direction and current speed
    switch (direction) {
        case 1: // North
            left_speed = left_base;
            right_speed = right_base;
            break;
        case 2: // North East
            left_speed = left_base * 0.5;       // Reduce left speed for turning
            right_speed = right_base;
            break;
        case 8: // North West
            left_speed = left_base;
            right_speed = right_base * 0.5;     // Reduce right speed for turning
            break;
        case 5: // South
            left_speed = -left_base;
            right_speed = -right_base;
            break;
        case 4: // South East
            left_speed = -left_base * 0.5;      // Reduce left speed for turning
            right_speed = -right_base;
            break;
        case 6: // South West
            left_speed = -left_base;
            right_speed = -right_base * 0.5;    // Reduce right speed for turning
            break;
        case 3: // East
            left_speed = -left_base;
            right_speed = right_base;
            break;
        case 7: // West
            left_speed = left_base;
            right_speed = -right_base;
            break;
        default: // Stop
            left_speed = 0;
            right_speed = 0;
            break;
    }

    bound_analog_values(&left_speed, &right_speed);
    set_dc_motor_output(left_speed, right_speed);
}

void set_dc_motor_output(int left_speed, int right_speed) {
    // Set motor speeds (handle direction with sign)
    if (left_speed >= 0) {
        analogWrite(MOTOR_A_IN1_PIN, left_speed);
        analogWrite(MOTOR_A_IN2_PIN, 0);
    } else {
        analogWrite(MOTOR_A_IN1_PIN, 0);
        analogWrite(MOTOR_A_IN2_PIN, -left_speed);
    }

    if (right_speed >= 0) {
        analogWrite(MOTOR_B_IN1_PIN, right_speed);
        analogWrite(MOTOR_B_IN2_PIN, 0);
    } else {
        analogWrite(MOTOR_B_IN1_PIN, 0);
        analogWrite(MOTOR_B_IN2_PIN, -right_speed);
    }
    // Add very small delay
    delay(2);
}

void stop_dc_motors() {
    // Stop both DC motors
    analogWrite(MOTOR_A_IN1_PIN, 0);
    analogWrite(MOTOR_A_IN2_PIN, 0);
    analogWrite(MOTOR_B_IN1_PIN, 0);
    analogWrite(MOTOR_B_IN2_PIN, 0);
}

void bound_analog_values(int* left_speed, int* right_speed) {
    // Reduce values exceeding ±255
    if (*left_speed > 255)
        *left_speed = 255;
    else if (*left_speed < -255) 
        *left_speed = -255;

    if (*right_speed > 255)
        *right_speed = 255;
    else if (*right_speed < -255)
        *right_speed = -255;

    // Increase values below minimum threshold to avoid stall
    if (*left_speed > 0 && *left_speed < MINIMUM_SPEED)
        *left_speed = MINIMUM_SPEED;
    else if (*left_speed < 0 && *left_speed > -MINIMUM_SPEED)
        *left_speed = -MINIMUM_SPEED;
    if (*right_speed > 0 && *right_speed < MINIMUM_SPEED)
        *right_speed = MINIMUM_SPEED;
    else if (*right_speed < 0 && *right_speed > -MINIMUM_SPEED)
        *right_speed = -MINIMUM_SPEED;
}