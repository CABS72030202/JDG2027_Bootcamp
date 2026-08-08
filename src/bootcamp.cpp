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

// Calibrated Speed Levels
#define BASE_SPEED          255
#define MINIMUM_SPEED       150
#define LEFT_CORRECTION     1.0
#define RIGHT_CORRECTION    1.0

void setup() {
    xy_init();
    setup_dc_motors(BASE_SPEED, MINIMUM_SPEED, LEFT_CORRECTION, RIGHT_CORRECTION);

}

void loop() {
    // Handle RemoteXY communication and update esp_controls state
    xy_handle_remotexy();
    xy_delay(1);

    // Update motor speeds based on esp_controls state
    set_dc_speed(esp_get_axis(ESP_JOYSTICK)->direction, esp_get_axis(ESP_JOYSTICK)->zone);
}