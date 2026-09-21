// esp_xy.h
// Created on: 2025-12-23
// Author: Sebastien Cabana
// Description: RemoteXY integration for ESP32 controller module,
//              replacing Raspberry Pi and F710 gamepad while keeping
//              same control interface.

#ifndef ESP_XY_H
#define ESP_XY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "esp-controls.h"
#include "motor.h"
#include "debug_options.h"

// Global Constants
#define XY_ANAL_SHIFT_VAL           0         // Shift value for analog inputs
#define XY_ANAL_MAX_VAL             100       // Maximum value for analog inputs
#define XY_AXIS_ZONE_COUNT          3         // Number of axis zones (including 0)
#define XY_JOYSTICK_THRESHOLD       10        // Threshold for joystick deadzone

// RemoteXY configuration (WiFi Point / SoftAP)
// NOTE: Must coexist with ESP-NOW which uses STA interface and a fixed channel
#ifndef REMOTEXY_WIFI_SSID
    #define REMOTEXY_WIFI_SSID          "ITR"
    #define REMOTEXY_WIFI_PASSWORD      "cremeuse123"
#endif
#define REMOTEXY_SERVER_PORT        6377
#define REMOTEXY_MODE__WIFI_POINT
#define REMOTEXY_WIFI_CHANNEL       6
#define REMOTEXY_WIFI_HIDDEN        0
#define REMOTEXY_WIFI_MAX_CONN      1

// Sending policy for ESP-NOW frames generated from RemoteXY state
// This helps prevent ESP_ERR_ESPNOW_NO_MEM by avoiding saturating the ESP-NOW TX queue
#define XY_TX_PERIOD_MS             20
#define XY_TX_SEND_ONLY_ON_CHANGE   1

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Functions Prototypes
void xy_init();                                                 // Initialize RemoteXY
void xy_handle_remotexy();                                      // Handle RemoteXY communication
void xy_delay(int ms);                                          // Delay function for RemoteXY
void update_xy_data();                                          // Update esp_controls data from XY remote state
int  xy_get_joystick_zone(int x_value, int y_value);            // Convert an analog value to a discrete zone
ESP_Direction xy_get_direction(int x, int y);                   // Get direction from x,y values (discrete view)
DriveCommand  xy_get_drive_command(int x_value, int y_value);   // Get polar command from x,y values (continuous view)

#endif // ESP_XY_H