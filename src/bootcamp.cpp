// bootcamp.cpp
// Created on: 2026-08-08
// Author: Sebastien Cabana
// Description: Main sketch for controlling simple robot with RemoteXY

// RemoteXY WiFi Point configuration
#define REMOTEXY_WIFI_SSID          "EQUIPE-00"
#define REMOTEXY_WIFI_PASSWORD      "mdp27"

// Includes
#include <Arduino.h>
#include "esp_xy.h"
#include "esp-controls.h"
#include "pins.h"
#include "motor.h"
#include "servo.h"

// Calibrated Speed Levels
const int   BASE_SPEED       = 255;
const int   MINIMUM_SPEED    = 150;
const float LEFT_CORRECTION  = 1.0f;
const float RIGHT_CORRECTION = 1.0f;

// Servo180 Configurations (Pin, Min, Max, Default, Step)
Servo180 servo1 (SERVO1_PIN, 0, 60,  0,   5);
Servo180 servo2 (SERVO2_PIN, 0, 100, 100, 5);

// Motor objects — IN1/IN2 are digital direction pins, EN is PWM speed pin
DCMotor rightMotor(MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, MOTOR_A_EN_PIN);
DCMotor leftMotor (MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, MOTOR_B_EN_PIN);

// High-level controller coordinating both motors
MotorController robot(leftMotor, rightMotor);

void setup() {
    xy_init();

    robot.configure(BASE_SPEED, MINIMUM_SPEED, LEFT_CORRECTION, RIGHT_CORRECTION);
    robot.begin();

    servo1.begin();
    servo2.begin();
}

void loop() {
    // Handle RemoteXY communication and update esp_controls state
    xy_handle_remotexy();
    xy_delay(1);

    ESP_Axis* stick = esp_get_axis(ESP_JOYSTICK);

    // Motors: use the continuous polar fields
    robot.drive({ stick->magnitude, stick->angle });

    // Servos: unchanged, still use button states
    servo1.process_commands(esp_get_button(ESP_BUTTON_ORANGE), esp_get_button(ESP_BUTTON_GREEN));
    servo2.process_commands(esp_get_button(ESP_BUTTON_RED),    esp_get_button(ESP_BUTTON_BLUE));
}