// esp-controls.cpp
// Created on: 2025-10-04
// Author: Sebastien Cabana
// Description: Implementation of ESP32 control state management.
//              Provides data decoding and state tracking for F710 controller input.

#include "esp-controls.h"

// Extern variables for each button
ESP_Button esp_button_a        = {ESP_BUTTON_A, 0, 0, 0, 0};
ESP_Button esp_button_b        = {ESP_BUTTON_B, 0, 0, 0, 0};
ESP_Button esp_button_x        = {ESP_BUTTON_X, 0, 0, 0, 0};
ESP_Button esp_button_y        = {ESP_BUTTON_Y, 0, 0, 0, 0};
ESP_Button esp_button_lb       = {ESP_BUTTON_LB, 0, 0, 0, 0};
ESP_Button esp_button_rb       = {ESP_BUTTON_RB, 0, 0, 0, 0};
ESP_Button esp_button_back     = {ESP_BUTTON_BACK, 0, 0, 0, 0};
ESP_Button esp_button_start    = {ESP_BUTTON_START, 0, 0, 0, 0};
ESP_Button esp_button_home     = {ESP_BUTTON_HOME, 0, 0, 0, 0};
ESP_Button esp_button_lstick   = {ESP_BUTTON_LSTICK, 0, 0, 0, 0};
ESP_Button esp_button_rstick   = {ESP_BUTTON_RSTICK, 0, 0, 0, 0};

// Extern variables for each axis
ESP_Axis esp_axis_lstick   = {ESP_AXIS_L, 0, ESP_NONE, &esp_button_lstick};
ESP_Axis esp_axis_rstick   = {ESP_AXIS_R, 0, ESP_NONE, &esp_button_rstick};
ESP_Axis esp_axis_cross    = {ESP_AXIS_C, 0, ESP_NONE, NULL};
ESP_Axis esp_axis_lt       = {ESP_AXIS_LT, 0, ESP_NONE, NULL};  // Only Y axis used
ESP_Axis esp_axis_rt       = {ESP_AXIS_RT, 0, ESP_NONE, NULL};  // Only Y axis used


std::string esp_get_direction_str(ESP_Direction dir) {
	switch (dir) {
        case 0:
            return "NONE";
        case 1:
            return "N";
        case 2:
            return "NE";
        case 3:
            return "E";
        case 4:
            return "SE";
        case 5:
            return "S";
        case 6:
            return "SW";
        case 7:
            return "W";
        case 8:
            return "NW";
        default:
            return "INVALID";
    }
}

void esp_unpack_data(const uint32_t* data) {
    uint32_t packet = *data;
    
    // Unpack button states (11 buttons, 1 bit each)
    update_button(&esp_button_a,        (packet >> 0) & 0x1);
    update_button(&esp_button_b,        (packet >> 1) & 0x1);
    update_button(&esp_button_x,        (packet >> 2) & 0x1);
    update_button(&esp_button_y,        (packet >> 3) & 0x1);
    update_button(&esp_button_lb,       (packet >> 4) & 0x1);
    update_button(&esp_button_rb,       (packet >> 5) & 0x1);
    update_button(&esp_button_back,     (packet >> 6) & 0x1);
    update_button(&esp_button_start,    (packet >> 7) & 0x1);
    update_button(&esp_button_lstick,   (packet >> 8) & 0x1);
    update_button(&esp_button_rstick,   (packet >> 9) & 0x1);

    // Unpack axis info
    // LStick - direction (4 bits) and zone (2 bits)
    esp_axis_lstick.direction = (ESP_Direction)((packet >> 10) & 0xF);
    esp_axis_lstick.zone      = (packet >> 14) & 0x3;
    
    // RStick - direction (4 bits) and zone (2 bits)
    esp_axis_rstick.direction = (ESP_Direction)((packet >> 16) & 0xF);
    esp_axis_rstick.zone      = (packet >> 20) & 0x3;
    
    // Cross - direction (4 bits) and zone (2 bits)
    esp_axis_cross.direction  = (ESP_Direction)((packet >> 22) & 0xF);
    esp_axis_cross.zone       = (packet >> 26) & 0x3;
    
    // LT zone (2 bits)
    esp_axis_lt.zone          = (packet >> 28) & 0x3;
    
    // RT zone (2 bits)
    esp_axis_rt.zone          = (packet >> 30) & 0x3;
}

void update_button(ESP_Button* button, bool input) {    
    // Handle digital buttons
    if(button->is_toggle == 0) {
        button->state = input;
        return;
    }

    // Handle toggle buttons with debouncing
    unsigned long now = millis();
    
    if(input) {
        // Check debounce: ignore press if we just released recently
        unsigned long time_since_release = now - button->last_release_ms;
        if(!button->is_pressed && time_since_release >= ESP_BUTTON_DEBOUNCE_MS) {
            // Toggle on rising edge (input high and wasn't pressed before)
            button->state = !button->state;
            button->is_pressed = 1;
        }
    } else {
        // Reset pressed flag when button is released
        if(button->is_pressed) {
            button->last_release_ms = now;
        }
        button->is_pressed = 0;
    }
}

void esp_print_states(void) {
    fprintf(stdout, "Buttons[A:%d B:%d X:%d Y:%d LB:%d RB:%d Back:%d Start:%d LStick:%d RStick:%d] Axes[LS:%s(z%d) RS:%s(z%d) Cross:%s(z%d) LT:z%d RT:z%d]\n",
            esp_button_a.state, esp_button_b.state, esp_button_x.state, esp_button_y.state, esp_button_lb.state, esp_button_rb.state, esp_button_back.state, esp_button_start.state, esp_button_lstick.state, esp_button_rstick.state,
            esp_get_direction_str(esp_axis_lstick.direction).c_str(), esp_axis_lstick.zone, esp_get_direction_str(esp_axis_rstick.direction).c_str(), esp_axis_rstick.zone, esp_get_direction_str(esp_axis_cross.direction).c_str(), esp_axis_cross.zone, esp_axis_lt.zone, esp_axis_rt.zone);
}

ESP_Button* esp_get_button(int button_id) {
    switch (button_id) {
    case ESP_BUTTON_A:
        return &esp_button_a;
    case ESP_BUTTON_B:
        return &esp_button_b;
    case ESP_BUTTON_X:
        return &esp_button_x;
    case ESP_BUTTON_Y:
        return &esp_button_y;
    case ESP_BUTTON_LB:
        return &esp_button_lb;
    case ESP_BUTTON_RB:
        return &esp_button_rb;
    case ESP_BUTTON_BACK:
        return &esp_button_back;
    case ESP_BUTTON_START:
        return &esp_button_start;
    case ESP_BUTTON_HOME:
        return &esp_button_home;
    case ESP_BUTTON_LSTICK:
        return &esp_button_lstick;
    case ESP_BUTTON_RSTICK:
        return &esp_button_rstick;
    default:
        return NULL;
    }
}   

ESP_Axis* esp_get_axis(int axis_id) {
    switch (axis_id) {
    case ESP_AXIS_L:
        return &esp_axis_lstick;
    case ESP_AXIS_R:
        return &esp_axis_rstick;
    case ESP_AXIS_C:
        return &esp_axis_cross;
    case ESP_AXIS_LT:
        return &esp_axis_lt;
    case ESP_AXIS_RT:
        return &esp_axis_rt;
    default:
        return NULL;
    }
}