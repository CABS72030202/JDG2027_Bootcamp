// esp_xy.cpp
// Created on: 2025-12-23
// Author: Sebastien Cabana
// Description: RemoteXY integration for ESP32 controller module, 
//              replacing Raspberry Pi and XY gamepad while keeping 
//              same control interface.

#include "esp_xy.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <RemoteXY.h>
#include <RemoteXYNet_WiFi.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 65 bytes V19 
  { 255,6,0,0,0,58,0,19,0,0,0,0,20,1,200,84,1,1,5,0,
  5,20,12,60,60,32,2,26,31,1,110,8,24,24,0,12,31,0,1,110,
  52,24,24,0,2,31,0,1,154,8,24,24,0,6,31,0,1,154,52,24,
  24,0,1,31,0 };

// Input variables for RemoteXY
struct {
  int8_t joystick_x;            // from -100 to 100
  int8_t joystick_y;            // from -100 to 100
  uint8_t button_green;         // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_orange;        // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_blue;          // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_red;           // =1 if button pressed, else =0, from 0 to 1
  uint8_t connect_flag;         // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)

// RemoteXY's built-in CRemoteXYNet_WiFiPoint forces WiFi.mode(WIFI_AP)
// ESP-NOW in this project is initialized in WIFI_STA mode on a fixed channel
// To make them coexist, we start the SoftAP in WIFI_AP_STA and force the
// channel to match ESP-NOW
class CRemoteXYNet_WiFiPoint_APSTA final : public CRemoteXYNet {
    public:
        CRemoteXYNet_WiFiPoint_APSTA(const char* ssid, const char* password, uint8_t channel)
                : CRemoteXYNet(), ssid(ssid), password(password), channel(channel), state(0) {
            // Keep STA interface alive (ESP-NOW uses WIFI_IF_STA)
            WiFi.mode(WIFI_AP_STA);
            WiFi.setSleep(false);
            WiFi.softAPdisconnect(true);

            // Ensure radio channel stays aligned with ESP-NOW.
            esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

            state = WiFi.softAP(ssid, password, channel, REMOTEXY_WIFI_HIDDEN, REMOTEXY_WIFI_MAX_CONN) ? 1 : 0;
        }

        uint8_t configured() override { return state; }

        CRemoteXYServer* createServer(uint16_t port) override {
            return new CRemoteXYServer_WiFi(this, port);
        }

        CRemoteXYClient* newClient() override { return new CRemoteXYClient_WiFi(); }

    private:
        const char* ssid;
        const char* password;
        uint8_t channel;
        uint8_t state;
};

void xy_init() {
    RemoteXYNet* net = new CRemoteXYNet_WiFiPoint_APSTA(
        REMOTEXY_WIFI_SSID, REMOTEXY_WIFI_PASSWORD, REMOTEXY_WIFI_CHANNEL);
    RemoteXYGui* gui = RemoteXYEngine.addGui (RemoteXY_CONF_PROGMEM, &RemoteXY);
    gui->addConnectionServer (net, REMOTEXY_SERVER_PORT);
}

void xy_handle_remotexy() {
    RemoteXYEngine.handler();
    update_xy_data();
}

void xy_delay(int ms) {
    RemoteXYEngine.delay(ms);
}

void update_xy_data() {
    // Update axis
    esp_get_axis(ESP_JOYSTICK)->zone = xy_get_joystick_zone(RemoteXY.joystick_x, RemoteXY.joystick_y);
    esp_get_axis(ESP_JOYSTICK)->direction = xy_get_direction(RemoteXY.joystick_x, RemoteXY.joystick_y);

    // Update buttons
    update_button(esp_get_button(ESP_BUTTON_GREEN), RemoteXY.button_green);
    update_button(esp_get_button(ESP_BUTTON_ORANGE), RemoteXY.button_orange);
    update_button(esp_get_button(ESP_BUTTON_BLUE), RemoteXY.button_blue);
    update_button(esp_get_button(ESP_BUTTON_RED), RemoteXY.button_red);

    if(ESP_PRINT_CONTROLS)
        esp_print_states();
}

int xy_get_joystick_zone(int x_value, int y_value) {
    // Calculate the magnitude of the joystick vector
    int magnitude = (int)sqrt(x_value * x_value + y_value * y_value);
    if (magnitude < XY_JOYSTICK_THRESHOLD)
        return 0; // Center zone
    int zone_size = (XY_ANAL_MAX_VAL * 0.5) / XY_AXIS_ZONE_COUNT;
    int zone = magnitude / (zone_size + 1);
    if (zone >= XY_AXIS_ZONE_COUNT)
        zone = XY_AXIS_ZONE_COUNT - 1;
    return zone;
}

ESP_Direction xy_get_direction(int x, int y) {
    const int DEADZONE = XY_JOYSTICK_THRESHOLD;
    int cx = x;
    int cy = -y;

    if (abs(cx) < DEADZONE && abs(cy) < DEADZONE) 
        return ESP_NONE;

    double angle = atan2((double)cy, (double)cx) * 180.0 / M_PI;
    if (angle < 0)
        angle += 360.0;

            if (angle >= 337.5 || angle < 22.5)
                return ESP_EAST;
            else if (angle >= 22.5 && angle < 67.5)
                return ESP_SOUTH_EAST;
            else if (angle >= 67.5 && angle < 112.5)
                return ESP_SOUTH;
            else if (angle >= 112.5 && angle < 157.5)
                return ESP_SOUTH_WEST;
            else if (angle >= 157.5 && angle < 202.5)
                return ESP_WEST;
            else if (angle >= 202.5 && angle < 247.5)
                return ESP_NORTH_WEST;
            else if (angle >= 247.5 && angle < 292.5)
                return ESP_NORTH;
            else // angle >= 292.5 && angle < 337.5
                return ESP_NORTH_EAST;
}