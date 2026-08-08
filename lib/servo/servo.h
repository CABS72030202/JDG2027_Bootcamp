// servo.h
// Created on: 2025-11-11
// Author: Sebastien Cabana
// Description: Servo motor control interface for precise positioning.
//              Manages servo motor initialization, position control and calibration.

#ifndef SERVO_H
#define SERVO_H

// Includes
#include "pins.h"
#include "esp-controls.h"
#include <ESP32Servo.h>
#include <vector>

using namespace std;

// Constants Definitions
#define CONTINUOUS_SERVO_CW_VALUE       0
#define CONTINUOUS_SERVO_CCW_VALUE      180
#define CONTINUOUS_SERVO_STOP_VALUE     95

// Struct Definitions
struct servo_pos {
    int id;
    int pos;
};
typedef struct servo_pos Servo_Pos;

// Class Definitions
class Servo180 {
private:

    // Private Attributes
    Servo servo;
    int pin;
    int min_pos;
    int max_pos;
    int default_pos;
    float step_size;
    bool inverted;                          // True when min_pos > max_pos for intuitive control
    vector<Servo_Pos> servo_pos_list;
    bool button_pressed;

    // Private Methods
    int bound(int pos);                     // Bound position within min/max and step size
    int get_servo_num(int pin);             // Get corresponding servo number based on pin

public:

    // Public Attributes
    int current_pos;
    float target_pos;

    // Constructors
    Servo180(int pin, int min_pos, int max_pos);
    Servo180(int pin, int min_pos, int max_pos, int default_pos, float step_size);

    // Public Methods
    void begin();                           // Initialize servo
    void sweep();                           // Sweep servo from min to max and back (blocking)
    void set_target_pos(float target);      // Set target position
    void increase_pos();                    // Increase target position by step size
    void decrease_pos();                    // Decrease target position by step size
    void update_pos();                      // Update current position towards target position
    void process_commands(ESP_Axis* axis, ESP_Direction increase_dir, ESP_Direction decrease_dir);          // Process commands based on axis input
    void process_commands(ESP_Button* increase_button, ESP_Button* decrease_button);                        // Process commands based on two button input
    void process_commands(ESP_Button* toggle_button);                                                       // Process commands based on toggle button
    void set_step_size(float step);         // Set step size
    void set_inverted(bool inv);            // Set inverted flag
    
    // Getters
    int get_min_pos();                      // Get minimum position
    int get_max_pos();                      // Get maximum position
    bool is_inverted();                     // Get inverted flag
    int get_default_pos();                  // Get default position

    // Position List Management
    void add_servo_pos(int id, int pos);    // Add servo position to list
    void add_servo_pos(Servo_Pos sp);       // Add servo position to list
    int get_pos_id(int pos);                // Get ID for a given position from list
    void set_target_pos_by_id(int id);      // Set target position using ID from list
    void set_next_pos();                    // Set target position to next in list
    void set_previous_pos();                // Set target position to previous in list
    void process_commands_list(ESP_Button* previous_button, ESP_Button* next_button);                       // Process commands based on two button input for position list navigation
    void process_commands_list(ESP_Button* next_button);                                                    // Process commands based on single button input for position list navigation

    // Destructor
    ~Servo180() { servo.detach(); }
};

class ServoContinuous {
private:

    // Private Attributes
    Servo servo;
    int pin;
    int timeout_ms;
    unsigned long start_time;
    bool timed_out;

    // Private Methods
    int get_servo_num(int pin);             // Get corresponding servo number based on pin
    void check_timeout();                   // Check and handle timeout condition

public:

    // Public Attributes
    int current_value;
    unsigned long current_time;

    // Constructors
    ServoContinuous(int pin, int timeout_ms);

    // Public Methods
    void begin();                           // Initialize servo
    void stop(bool reset_timeout = true);   // Stop the servo
    void rotate_cw();                       // Rotate clockwise
    void rotate_ccw();                      // Rotate counter-clockwise
    void update();                          // Update servo state
    void process_commands(ESP_Axis* axis, ESP_Direction cw_dir, ESP_Direction ccw_dir);          // Process commands based on axis input
    void process_commands(ESP_Button* cw_button, ESP_Button* ccw_button);                        // Process commands based on two button input

    
    // Destructor
    ~ServoContinuous() { servo.detach(); }
};

#endif // SERVO_H