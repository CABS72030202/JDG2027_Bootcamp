// motor.h
// Created on: 2025-10-28
// Author: Sebastien Cabana
// Description: Universal motor control interface supporting multiple motor types.
//              Handles DC motors, mecanum wheels, and stepper motor operations.

#ifndef MOTOR_H
#define MOTOR_H

// Includes
#include <Arduino.h>
#include <pins.h>

// Global Constants
#define MOTOR_DEBUG                         0                   // Enable debug output for motor control

#define MAX_SPEED                           3                   // Maximum speed levels
const int SPEED_MULTIPLIERS[MAX_SPEED] =    {60, 80, 100};      // Percentage for speed levels

// Motor Correction Factors
extern int BASE_SPEED;                  // Base analog speed for motors (0-255)
extern int MINIMUM_SPEED;               // Minimum analog speed to overcome motor stall (0-255)
extern float LEFT_CORRECTION;           // Correction factor for left motor (0.1-2.0)
extern float RIGHT_CORRECTION;          // Correction factor for right motor (0.1-2.0)

// Global Variables
extern int current_speed;               // Current speed level index (0 to MAX_SPEED-1)
extern int current_direction;           // Current direction code (0 to 8)
extern int left_speed;                  // Current speed for the left motor (analog value)
extern int right_speed;                 // Current speed for the right motor (analog value)

// Function Prototypes
void setup_dc_motors(int base, int min, int left_correction, int right_correction);     // Initialize DC motor control pins
void set_dc_speed(int direction, int speed_level);                                      // Set DC motor speed based on direction and speed level
void set_dc_motor_output(int left_speed, int right_speed);                              // Apply speed values to DC motors
void stop_dc_motors();                                                                  // Stop both DC motors
void bound_analog_values(int* left_speed, int* right_speed);                            // Ensure speed values are within valid range

#endif // MOTOR_H