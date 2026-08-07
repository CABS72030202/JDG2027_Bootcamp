// main.cpp
// Created on: 2026-08-08
// Author: Sebastien Cabana
// Description: 

// Includes
#include <Arduino.h>
#include "esp_xy.h"
#include "esp-controls.h"
#include "pins.h"

void setup() {
    xy_init();
}

void loop() {
    // Handle RemoteXY communication and update esp_controls state
    xy_handle_remotexy();
    xy_delay(1);
}