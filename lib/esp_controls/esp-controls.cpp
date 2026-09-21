// esp-controls.cpp
// Created on: 2025-10-04
// Author: Sebastien Cabana
// Description: ESP32 control state management and data decoding interface.
//              Handles controller data received through RemoteXY.

#include "esp-controls.h"

// Extern variables for each button
static ESP_Button esp_button_green   = {ESP_BUTTON_GREEN, 0, 0, 0, 0};
static ESP_Button esp_button_orange  = {ESP_BUTTON_ORANGE, 0, 0, 0, 0};
static ESP_Button esp_button_blue    = {ESP_BUTTON_BLUE, 0, 0, 0, 0};
static ESP_Button esp_button_red     = {ESP_BUTTON_RED, 0, 0, 0, 0};

// Extern variable for the joystick (both discrete and polar views)
static ESP_Axis esp_joystick = {ESP_JOYSTICK, 0, ESP_NONE, nullptr, 0.0f, 0.0f};

std::string esp_get_direction_str(ESP_Direction dir) {
    switch (dir) {
        case 0: return "NONE";
        case 1: return "N";
        case 2: return "NE";
        case 3: return "E";
        case 4: return "SE";
        case 5: return "S";
        case 6: return "SW";
        case 7: return "W";
        case 8: return "NW";
        default: return "INVALID";
    }
}

void update_button(ESP_Button* button, bool input) {
    // Handle digital buttons
    if (button->is_toggle == 0) {
        button->state = input;
        return;
    }

    // Handle toggle buttons with debouncing
    unsigned long now = millis();

    if (input) {
        // Check debounce: ignore press if we just released recently
        unsigned long time_since_release = now - button->last_release_ms;
        if (!button->is_pressed && time_since_release >= ESP_BUTTON_DEBOUNCE_MS) {
            // Toggle on rising edge (input high and wasn't pressed before)
            button->state = !button->state;
            button->is_pressed = 1;
        }
    } else {
        // Reset pressed flag when button is released
        if (button->is_pressed) {
            button->last_release_ms = now;
        }
        button->is_pressed = 0;
    }
}

void esp_print_states(void) {
    fprintf(stdout, "Buttons[Green:%d Orange:%d Blue:%d Red:%d] "
                    "Joystick[%s(z%d) r=%.2f th=%.2f]\n",
            esp_button_green.state, esp_button_orange.state,
            esp_button_blue.state,  esp_button_red.state,
            esp_get_direction_str(esp_joystick.direction).c_str(),
            esp_joystick.zone,
            esp_joystick.magnitude, esp_joystick.angle);
}

ESP_Button* esp_get_button(int button_id) {
    switch (button_id) {
    case ESP_BUTTON_GREEN:
        return &esp_button_green;
    case ESP_BUTTON_ORANGE:
        return &esp_button_orange;
    case ESP_BUTTON_BLUE:
        return &esp_button_blue;
    case ESP_BUTTON_RED:
        return &esp_button_red;
    default:
        return NULL;
    }
}

ESP_Axis* esp_get_axis(int axis_id) {
    switch (axis_id) {
    case ESP_JOYSTICK:
        return &esp_joystick;
    default:
        return NULL;
    }
}