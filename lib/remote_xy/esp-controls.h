// esp-controls.h
// Created on: 2025-10-04
// Author: Sebastien Cabana
// Description: ESP32 control state management and data decoding interface.
//              Handles controller data received through RemoteXY.

#ifndef ESP_CONTROLS_H
#define ESP_CONTROLS_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Arduino.h>
#include "debug_options.h"

// Digital Button IDs
enum esp_button_id {
    ESP_BUTTON_GREEN,
    ESP_BUTTON_ORANGE,
    ESP_BUTTON_BLUE,
    ESP_BUTTON_RED
};
typedef enum esp_button_id ESP_Button_ID;

// Analog Button IDs (Axis)
enum esp_axis_id {
    ESP_JOYSTICK
};
typedef enum esp_axis_id ESP_Axis_ID;

// Enums
enum esp_direction {
    ESP_NONE,
    ESP_NORTH,
    ESP_NORTH_EAST,
    ESP_EAST,
    ESP_SOUTH_EAST,
    ESP_SOUTH,
    ESP_SOUTH_WEST,
    ESP_WEST,
    ESP_NORTH_WEST
};
typedef enum esp_direction ESP_Direction;

// Structs
struct esp_button {
    int id;                         // Button ID
    int state;                      // 1 if active, 0 if inactive
    int is_toggle;                  // 1 if toggle button, 0 if not
    int is_pressed;                 // 1 if button was just pressed, 0 otherwise
    unsigned long last_release_ms;  // Timestamp of last release (for debouncing)
};
typedef struct esp_button ESP_Button;

struct esp_axis {
    int id;                         // Axis ID
    int zone;                       // Current zone
    ESP_Direction direction;        // Current direction (for sticks)
    ESP_Button* associated_button;  // Pointer to associated button (if any)
};
typedef struct esp_axis ESP_Axis;

// Extern variables for each button
extern ESP_Button esp_button_green;
extern ESP_Button esp_button_orange;
extern ESP_Button esp_button_blue;
extern ESP_Button esp_button_red;

// Extern variables for each axis
extern ESP_Axis esp_joystick;

// Global Constants
#define ESP_BUTTON_DEBOUNCE_MS  50      // Debounce delay in milliseconds

// Global variables

// Function prototypes
std::string esp_get_direction_str(ESP_Direction dir);   // Get string representation of direction
void update_button(ESP_Button* button, bool input);     // Update button state with toggle logic
void esp_print_states(void);                            // Print current states of all buttons and axes

// Get functions for buttons and axes
ESP_Button* esp_get_button(int button_id);              // Get pointer to button struct
ESP_Axis* esp_get_axis(int axis_id);                    // Get pointer to axis struct

#endif // ESP_CONTROLS_H