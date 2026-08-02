// servoContinuous.cpp    
// Created on: 2025-11-13
// Author: Sebastien Cabana
// Description: Implementation of servo motor control functions.
//              Provides PWM signal generation and position control algorithms.

#include <servo.h>

// ======================================================================
// PRIVATE METHODS
// ======================================================================

int ServoContinuous::get_servo_num(int pin) {
    switch(pin) {
        case SERVO1_PIN:
            return 1;
        case SERVO2_PIN:
            return 2;
        case SERVO3_PIN:
            return 3;
        case SERVO4_PIN:
            return 4;
        case SERVO5_PIN:
            return 5;
        case SERVO6_PIN:
            return 6;    
        default:
            return -1;
    }
}

void ServoContinuous::check_timeout() {
    if (this->timeout_ms < 0)
        return;             // No timeout configured
    this->current_time = millis();
    if (this->current_time - this->start_time > this->timeout_ms) {
        if (SERVO_DEBUG)
            fprintf(stderr, "(Timeout occurred) ");
        this->timed_out = true;
        this->stop(false);  // Don't reset timed_out flag when stopping due to timeout
    }
}

// ======================================================================
// CONSTRUCTORS
// ======================================================================

ServoContinuous::ServoContinuous(int pin, int timeout_ms) {
    this->servo.attach(pin);
    this->pin = pin;
    this->timeout_ms = timeout_ms;
    this->current_value = CONTINUOUS_SERVO_STOP_VALUE;
    this->current_time = 0;
    this->start_time = 0;
    this->timed_out = false;
    this->begin();
}

// ======================================================================
// PUBLIC METHODS
// ======================================================================

void ServoContinuous::begin() {
    this->servo.attach(this->pin);
    this->servo.write(CONTINUOUS_SERVO_STOP_VALUE);
}

void ServoContinuous::stop(bool reset_timeout) {
    this->start_time = 0;
    this->current_value = CONTINUOUS_SERVO_STOP_VALUE;
    if (reset_timeout)
        this->timed_out = false;
    if(SERVO_DEBUG)
        fprintf(stderr, "Servo %d: Stopped.\n", get_servo_num(this->pin));
}

void ServoContinuous::rotate_cw() {
    // Don't start if we're in timeout state
    if (this->timed_out) {
        return;
    }
    // Only reset timer if not already rotating CW
    if (this->current_value != CONTINUOUS_SERVO_CW_VALUE) {
        this->start_time = millis();
    }
    this->current_value = CONTINUOUS_SERVO_CW_VALUE;
    if(SERVO_DEBUG)
        fprintf(stderr, "Servo %d: Rotating clockwise.\n", get_servo_num(this->pin));
}

void ServoContinuous::rotate_ccw() {
    // Don't start if we're in timeout state
    if (this->timed_out) {
        return;
    }
    // Only reset timer if not already rotating CCW
    if (this->current_value != CONTINUOUS_SERVO_CCW_VALUE) {
        this->start_time = millis();
    }
    this->current_value = CONTINUOUS_SERVO_CCW_VALUE;
    if(SERVO_DEBUG)
        fprintf(stderr, "Servo %d: Rotating counter-clockwise.\n", get_servo_num(this->pin));
}

void ServoContinuous::update() {
    if(this->start_time != 0)
        this->check_timeout();
    this->servo.write(this->current_value);
}

void ServoContinuous::process_commands(ESP_Axis* axis, ESP_Direction cw_dir, ESP_Direction ccw_dir) {
    if (axis->direction == cw_dir)
        this->rotate_cw();
    else if (axis->direction == ccw_dir)
        this->rotate_ccw();
    else if (this->current_value != CONTINUOUS_SERVO_STOP_VALUE || this->timed_out)
        this->stop(true);  // Reset timeout flag when axis is released
    this->update();
}

void ServoContinuous::process_commands(ESP_Button* cw_button, ESP_Button* ccw_button) {
    if (cw_button->state == 1)
        this->rotate_cw();
    else if (ccw_button->state == 1)
        this->rotate_ccw();
    else 
        this->stop(true);  // Reset timeout flag when buttons are released
    this->update();
}