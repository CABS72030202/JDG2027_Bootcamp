// bootcamp.cpp
// Created on: 2026-08-08
// Author: Sebastien Cabana
// Description: Main sketch for controlling simple robot with RemoteXY

// Includes
#include <Arduino.h>
#include "esp_xy.h"
#include "esp-controls.h"
#include "pins.h"
#include "motor.h"
#include "servo.h"

// Calibrated Speed Levels
#define BASE_SPEED          255
#define MINIMUM_SPEED       150
#define LEFT_CORRECTION     1.0
#define RIGHT_CORRECTION    1.0

// Servo180 Configurations (Pin, Min, Max, Default, Step)
Servo180 servo1 (SERVO1_PIN, 0, 60, 0, 5);    
Servo180 servo2 (SERVO2_PIN, 0, 100, 100, 5);

void setup() {
    xy_init();
    setup_dc_motors(BASE_SPEED, MINIMUM_SPEED, LEFT_CORRECTION, RIGHT_CORRECTION);
    servo1.begin();
    servo2.begin();
}

void loop() {
    // Handle RemoteXY communication and update esp_controls state
    xy_handle_remotexy();
    xy_delay(1);

    // Update motor speeds based on esp_controls state
    set_dc_speed(esp_get_axis(ESP_JOYSTICK)->direction, esp_get_axis(ESP_JOYSTICK)->zone);

    // Update servo positions based on button states
    servo1.process_commands(esp_get_button(ESP_BUTTON_ORANGE), esp_get_button(ESP_BUTTON_GREEN));
    servo2.process_commands(esp_get_button(ESP_BUTTON_RED), esp_get_button(ESP_BUTTON_BLUE));
}