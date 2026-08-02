// controller_xy.cpp
// Created on: 2025-12-23
// Author: Sebastien Cabana
// Description: ESP32 transmitter/receiver module implementation.
//              Handles wireless message transmission between machine components
//              and receives remote control commands via RemoteXY, bypassing
//              Raspberry Pi and F710 gamepad.

// Includes
#include <wireless-com.h>
#include <esp_xy.h>
#include <pins.h>
#include <ws2811.h>

void setup() {
    init_esp_now();
    xy_init();
    init_ws2811();
}

void loop() {
    uint32_t data;

    xy_handle_remotexy();
    if (xy_get_data_to_send(&data))
        esp_now_device.send_message((uint8_t*)&data, sizeof(data));
    update_selected_board();
    xy_delay(1);
}