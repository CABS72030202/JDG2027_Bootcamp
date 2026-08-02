// esp_xy.cpp
// Created on: 2025-12-23
// Author: Sebastien Cabana
// Description: RemoteXY integration for ESP32 controller module, 
//              replacing Raspberry Pi and XY gamepad while keeping 
//              same control interface.

#include <esp_xy.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <RemoteXY.h>
#include <RemoteXYNet_WiFi.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 266 bytes V19 
  { 255,19,0,0,0,3,1,19,0,0,0,0,0,1,200,92,1,1,27,0,
  1,114,2,19,14,3,22,31,0,5,24,12,33,33,32,8,26,31,5,50,
  54,33,33,32,234,26,31,5,119,55,33,33,32,22,26,31,1,76,36,14,
  14,0,26,31,0,1,94,36,14,14,0,30,31,0,1,112,36,14,14,0,
  26,31,0,1,156,2,20,20,0,79,31,0,1,156,42,20,20,0,38,31,
  0,1,136,22,20,20,0,191,31,0,1,176,22,20,20,0,134,31,0,1,
  84,64,14,14,0,234,31,0,1,104,64,14,14,0,22,31,0,4,87,0,
  9,27,16,233,26,4,106,0,9,27,16,22,26,1,69,2,19,14,3,233,
  31,0,129,182,27,8,12,64,8,65,0,129,163,47,8,12,64,8,66,0,
  129,162,7,8,12,64,8,89,0,129,142,27,8,12,64,8,88,0,129,80,
  38,7,12,64,31,60,0,129,116,38,7,12,64,31,62,0,129,75,6,9,
  7,64,8,76,66,0,129,119,6,9,7,64,8,82,66,0,129,95,51,12,
  4,64,8,72,79,77,69,0,129,89,68,4,7,64,8,76,0,129,109,68,
  5,7,64,8,82,0 };

// Input variables for RemoteXY
struct {
  uint8_t xy_button_rb;     // =1 if button pressed, else =0, from 0 to 1
  int8_t xy_axis_cross_x;   // from -100 to 100
  int8_t xy_axis_cross_y;   // from -100 to 100
  int8_t xy_axis_lstick_x;  // from -100 to 100
  int8_t xy_axis_lstick_y;  // from -100 to 100
  int8_t xy_axis_rstick_x;  // from -100 to 100
  int8_t xy_axis_rstick_y;  // from -100 to 100
  uint8_t xy_button_back;   // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_home;   // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_start;  // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_y;      // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_b;      // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_x;      // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_a;      // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_lstick; // =1 if button pressed, else =0, from 0 to 1
  uint8_t xy_button_rstick; // =1 if button pressed, else =0, from 0 to 1
  int8_t xy_axis_lt;        // from 0 to 100
  int8_t xy_axis_rt;        // from 0 to 100
  uint8_t xy_button_lb;     // =1 if button pressed, else =0, from 0 to 1

  uint8_t connect_flag;     // =1 if wire connected, else =0
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
    esp_get_axis(ESP_AXIS_C)->zone = xy_get_joystick_zone(RemoteXY.xy_axis_cross_x, RemoteXY.xy_axis_cross_y);
    esp_get_axis(ESP_AXIS_C)->direction = xy_get_direction(RemoteXY.xy_axis_cross_x, RemoteXY.xy_axis_cross_y);
    esp_get_axis(ESP_AXIS_L)->zone = xy_get_joystick_zone(RemoteXY.xy_axis_lstick_x, RemoteXY.xy_axis_lstick_y);
    esp_get_axis(ESP_AXIS_L)->direction = xy_get_direction(RemoteXY.xy_axis_lstick_x, RemoteXY.xy_axis_lstick_y);
    esp_get_axis(ESP_AXIS_R)->zone = xy_get_joystick_zone(RemoteXY.xy_axis_rstick_x, RemoteXY.xy_axis_rstick_y);
    esp_get_axis(ESP_AXIS_R)->direction = xy_get_direction(RemoteXY.xy_axis_rstick_x, RemoteXY.xy_axis_rstick_y);

    // Update pivots
    esp_get_axis(ESP_AXIS_LT)->zone = xy_get_pivot_zone(RemoteXY.xy_axis_lt);
    esp_get_axis(ESP_AXIS_RT)->zone = xy_get_pivot_zone(RemoteXY.xy_axis_rt);

    // Update buttons
    update_button(esp_get_button(ESP_BUTTON_BACK), RemoteXY.xy_button_back);
    update_button(esp_get_button(ESP_BUTTON_HOME), RemoteXY.xy_button_home);
    update_button(esp_get_button(ESP_BUTTON_START), RemoteXY.xy_button_start);
    update_button(esp_get_button(ESP_BUTTON_Y), RemoteXY.xy_button_y);
    update_button(esp_get_button(ESP_BUTTON_B), RemoteXY.xy_button_b);
    update_button(esp_get_button(ESP_BUTTON_X), RemoteXY.xy_button_x);
    update_button(esp_get_button(ESP_BUTTON_A), RemoteXY.xy_button_a);
    update_button(esp_get_button(ESP_BUTTON_LSTICK), RemoteXY.xy_button_lstick);
    update_button(esp_get_button(ESP_BUTTON_RSTICK), RemoteXY.xy_button_rstick);
    update_button(esp_get_button(ESP_BUTTON_LB), RemoteXY.xy_button_lb);
    update_button(esp_get_button(ESP_BUTTON_RB), RemoteXY.xy_button_rb);

    if(ESP_PRINT_CONTROLS)
        esp_print_states();
}

unsigned int xy_format_data() {
    unsigned int result = 0;
    // Pack button states (11 buttons, 1 bit each)
    result |= (esp_get_button(ESP_BUTTON_A)->state      & 0x1) << 0;
    result |= (esp_get_button(ESP_BUTTON_B)->state      & 0x1) << 1;
    result |= (esp_get_button(ESP_BUTTON_X)->state      & 0x1) << 2;
    result |= (esp_get_button(ESP_BUTTON_Y)->state      & 0x1) << 3;
    result |= (esp_get_button(ESP_BUTTON_LB)->state     & 0x1) << 4;
    result |= (esp_get_button(ESP_BUTTON_RB)->state     & 0x1) << 5;
    result |= (esp_get_button(ESP_BUTTON_BACK)->state   & 0x1) << 6;
    result |= (esp_get_button(ESP_BUTTON_START)->state  & 0x1) << 7;
    //result |= (esp_get_button(ESP_BUTTON_HOME)->state & 0x1) << 32;     // Home button omitted because of 32-bit limit
    result |= (esp_get_button(ESP_BUTTON_LSTICK)->state & 0x1) << 8;
    result |= (esp_get_button(ESP_BUTTON_RSTICK)->state & 0x1) << 9;

    // Pack axis info (direction: 4 bits, zone: 2 bits each)
    // LStick
    result |= (esp_get_axis(ESP_AXIS_L)->direction      & 0xF) << 10;
    result |= (esp_get_axis(ESP_AXIS_L)->zone           & 0x3) << 14;
    // RStick
    result |= (esp_get_axis(ESP_AXIS_R)->direction      & 0xF) << 16;
    result |= (esp_get_axis(ESP_AXIS_R)->zone           & 0x3) << 20;
    // Cross
    result |= (esp_get_axis(ESP_AXIS_C)->direction      & 0xF) << 22;
    result |= (esp_get_axis(ESP_AXIS_C)->zone           & 0x3) << 26;
    // LT zone (2 bits)
    result |= (esp_get_axis(ESP_AXIS_LT)->zone          & 0x3) << 28;
    // RT zone (2 bits)
    result |= (esp_get_axis(ESP_AXIS_RT)->zone          & 0x3) << 30;

    if (XY_DEBUG)
        fprintf(stderr, "[XY] Formatted data: 0x%08X\n", result);
    return result;
}

bool xy_get_data_to_send(uint32_t* data) {
    static uint32_t last_send_ms = 0;
    static uint32_t last_data = 0;

    // Check if connected to RemoteXY
    if (!RemoteXY.connect_flag) {
        if (XY_DEBUG)
            fprintf(stderr, "[XY] ERROR. Cannot get controller data: RemoteXY is not connected.\n");
        return false;
    }

    // Format the current data
    uint32_t current_data = xy_format_data();

    // Apply rate limiting and change detection
    const uint32_t now = millis();
    const bool time_ok = (uint32_t)(now - last_send_ms) >= (uint32_t)XY_TX_PERIOD_MS;
    const bool changed = (current_data != last_data);

    if (time_ok && (!XY_TX_SEND_ONLY_ON_CHANGE || changed)) {
        last_send_ms = now;
        last_data = current_data;
        *data = current_data;
        return true;
    }
    return false;
}

int xy_get_pivot_zone(int value) {
    value += XY_ANAL_SHIFT_VAL;
    int zone_size = XY_ANAL_MAX_VAL / XY_AXIS_ZONE_COUNT;
    int zone = value / zone_size;
    if (zone >= XY_AXIS_ZONE_COUNT)
        zone = XY_AXIS_ZONE_COUNT - 1;
    return zone;
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