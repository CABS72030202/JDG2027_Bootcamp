// esp-controls.h
// Created on: 2025-10-04
// Author: Sebastien Cabana
// Description: ESP32 control state management and data decoding interface.
//              Handles F710 controller data received through ESP-NOW protocol.

#ifndef ESP_CONTROLS_H
#define ESP_CONTROLS_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <debug_options.h>
#include <Arduino.h>

// Digital Button IDs
#define ESP_BUTTON_A       0
#define ESP_BUTTON_B       1
#define ESP_BUTTON_X       2
#define ESP_BUTTON_Y       3
#define ESP_BUTTON_LB      4
#define ESP_BUTTON_RB      5
#define ESP_BUTTON_BACK    6
#define ESP_BUTTON_START   7
#define ESP_BUTTON_HOME    8
#define ESP_BUTTON_LSTICK  9
#define ESP_BUTTON_RSTICK  10

// Analog Button IDs (Axis)
#define ESP_AXIS_LT        2
#define ESP_AXIS_RT        5
#define ESP_AXIS_L         1
#define ESP_AXIS_R         3
#define ESP_AXIS_C         6

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
extern ESP_Button esp_button_a;
extern ESP_Button esp_button_b;
extern ESP_Button esp_button_x;
extern ESP_Button esp_button_y;
extern ESP_Button esp_button_lb;
extern ESP_Button esp_button_rb;
extern ESP_Button esp_button_back;
extern ESP_Button esp_button_start;
extern ESP_Button esp_button_home;
extern ESP_Button esp_button_lstick;
extern ESP_Button esp_button_rstick;

// Extern variables for each axis
extern ESP_Axis esp_axis_lstick;
extern ESP_Axis esp_axis_rstick;
extern ESP_Axis esp_axis_cross;
extern ESP_Axis esp_axis_lt;
extern ESP_Axis esp_axis_rt;

// Global Constants
#define ESP_BUTTON_DEBOUNCE_MS 50  // Debounce delay in milliseconds

// Global variables

// Function prototypes
std::string esp_get_direction_str(ESP_Direction dir);   // Get string representation of direction
void esp_unpack_data(const uint32_t* data);             // Unpack controller data from 32 bits array
void update_button(ESP_Button* button, bool input);     // Update button state with toggle logic
void esp_print_states(void);                            // Print current states of all buttons and axes

// Get functions for buttons and axes
ESP_Button* esp_get_button(int button_id);              // Get pointer to button struct
ESP_Axis* esp_get_axis(int axis_id);                    // Get pointer to axis struct

#endif // ESP_CONTROLS_H