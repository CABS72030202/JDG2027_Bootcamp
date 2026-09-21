// servo180.cpp    
// Created on: 2025-11-11
// Author: Sebastien Cabana
// Description: Implementation of servo motor control functions.
//              Provides PWM signal generation and position control algorithms.

#include <servo.h>

// ======================================================================
// PRIVATE METHODS
// ======================================================================

int Servo180::bound(int pos) {
    // Normalize range so bound works even when min_pos > max_pos
    int low = (min_pos < max_pos) ? min_pos : max_pos;
    int high = (min_pos < max_pos) ? max_pos : min_pos;

    // Clamp into [low, high]
    if (pos < low) pos = low;
    if (pos > high) pos = high;

    // Ensure target position is a multiple of step size from default position
    float offset = pos - default_pos;
    int steps = 0;
    if (step_size != 0) steps = (int)(offset / step_size);
    pos = default_pos + (int)(steps * step_size);

    // After aligning to step increments, re-clamp into valid range
    if (pos < low) pos = low;
    if (pos > high) pos = high;

    return pos;
}

int Servo180::get_servo_num(int pin) {
    switch(pin) {
        case SERVO1_PIN:
            return 1;
        case SERVO2_PIN:
            return 2;
        default:
            return -1;
    }
}

// ======================================================================
// CONSTRUCTORS
// ======================================================================

Servo180::Servo180(int pin, int min_pos, int max_pos) {
    this->pin = pin;
    this->min_pos = min_pos;
    this->max_pos = max_pos;
    // No step means only two possible positions (min and max)
    this->step_size = max_pos - min_pos;
    // Inverted indicates the logical range direction: true when min_pos > max_pos
    this->inverted = (min_pos > max_pos);
    if(this->inverted)
        this->default_pos = max_pos;
    else
        this->default_pos = min_pos;
    this->current_pos = default_pos;
    this->target_pos = (float)default_pos;
    this->begin();
    this->button_pressed = false;
}

Servo180::Servo180(int pin, int min_pos, int max_pos, int default_pos, float step_size) {
    this->pin = pin;
    this->min_pos = min_pos;
    this->max_pos = max_pos;
    this->default_pos = default_pos;
    this->step_size = step_size;
    // Inverted indicates the logical range direction: true when min_pos > max_pos
    this->inverted = (min_pos > max_pos);
    this->current_pos = default_pos;
    this->target_pos = (float)default_pos;
    this->begin();
    this->button_pressed = false;
}

// ======================================================================
// PUBLIC METHODS
// ======================================================================

void Servo180::begin() {
    servo.attach(pin);
    // Write the physical position corresponding to the logical default
    int phys = this->inverted ? (this->min_pos + this->max_pos - this->default_pos) : this->default_pos;
    servo.write(phys);
    if (SERVO_DEBUG)
        fprintf(stderr, "Servo %d: Initialized successfully. Min = %d | Max = %d | Default = %d | Step = %.1f | Inverted = %s\n", get_servo_num(this->pin), min_pos, max_pos, default_pos, step_size, this->inverted ? "true" : "false");

}

void Servo180::sweep() {
    // Sweep from min_pos to max_pos following step sign (supports inverted ranges)
    int dir = (max_pos >= min_pos) ? 1 : -1;
    float step = dir * step_size;

    // Move towards max_pos
    while (current_pos != max_pos) {
        float next_target = target_pos + step;
        this->set_target_pos(next_target);
        this->update_pos();
        delay((int)abs(step_size));
    }

    // Pause at max position
    delay(1000);

    // Move back towards min_pos
    while (current_pos != min_pos) {
        float next_target = target_pos - step;
        this->set_target_pos(next_target);
        this->update_pos();
        delay((int)abs(step_size));
    }
}

void Servo180::set_target_pos(float target) {
    // Clamp target to valid range
    int low = (min_pos < max_pos) ? min_pos : max_pos;
    int high = (min_pos < max_pos) ? max_pos : min_pos;
    if (target < low) target = low;
    if (target > high) target = high;
    
    this->target_pos = target;
    if (SERVO_DEBUG && this->current_pos != (int)this->target_pos)
        fprintf(stderr, "Servo %d: Target position set to %.2f \n", get_servo_num(this->pin), this->target_pos);
}

void Servo180::increase_pos() {
    if (!this->inverted)
        this->set_target_pos(this->target_pos + step_size);
    else
        this->set_target_pos(this->target_pos - step_size);
}

void Servo180::decrease_pos() {
    if (!this->inverted)
        this->set_target_pos(this->target_pos - step_size);
    else
        this->set_target_pos(this->target_pos + step_size);
}

void Servo180::update_pos() {
    // Only update current_pos when target_pos crosses an integer boundary
    int new_pos = (int)target_pos;
    
    // Check if we've crossed an integer boundary
    if (new_pos != current_pos) {
        current_pos = new_pos;
        // Map logical current_pos to physical servo angle when inverted
        int phys = this->inverted ? (this->min_pos + this->max_pos - this->current_pos) : this->current_pos;
        servo.write(phys);
    }
}

void Servo180::process_commands(ESP_Axis* axis, ESP_Direction increase_dir, ESP_Direction decrease_dir) {
    if (axis->zone != 0) {
        if (axis->direction == increase_dir)
            this->increase_pos();
        else if (axis->direction == decrease_dir)
            this->decrease_pos();
    }
    this->update_pos();
}

void Servo180::process_commands(ESP_Button* increase_button, ESP_Button* decrease_button) {
    if (increase_button->state == 1)
        this->increase_pos();
    else if (decrease_button->state == 1)
        this->decrease_pos();
    this->update_pos();
}   

void Servo180::process_commands(ESP_Button* toggle_button) {
    if (toggle_button->state == 1)
        this->increase_pos();
    else
        this->decrease_pos();
    this->update_pos();
}

// ======================================================================
// POSITION LIST MANAGEMENT
// ======================================================================

void Servo180::add_servo_pos(int id, int pos) {
    // Check if ID already exists
    for (const auto& sp : servo_pos_list) {
        if (sp.id == id) {
            if (SERVO_DEBUG)
                fprintf(stderr, "Servo %d: Servo position with ID %d already exists. Skipping addition.\n", get_servo_num(this->pin), id);
            return;
        }
    }

    // Add new servo position
    int bound_pos = bound(pos);
    Servo_Pos new_pos = {id, bound_pos};
    servo_pos_list.push_back(new_pos);

    if (SERVO_DEBUG)
        fprintf(stderr, "Servo %d: Added servo position: ID=%d, Pos=%d\n", get_servo_num(pin), id, bound_pos);
}

void Servo180::add_servo_pos(Servo_Pos sp) {
    this->add_servo_pos(sp.id, sp.pos);
}

int Servo180::get_pos_id(int pos) {
    for (const auto& sp : servo_pos_list) {
        if (sp.pos == pos) {
            if (SERVO_DEBUG)
                fprintf(stderr, "Servo %d: Found ID %d for position %d\n", get_servo_num(this->pin), sp.id, pos);
            return sp.id;
        }
    }
    if (SERVO_DEBUG)
        fprintf(stderr, "Servo %d: Position %d not found in servo position list.\n", get_servo_num(this->pin), pos);
    return -1; // Indicate not found
}

void Servo180::set_target_pos_by_id(int id) {
    for (const auto& sp : servo_pos_list) {
        if (sp.id == id) {
            this->set_target_pos(sp.pos);
            return;
        }
    }
    if (SERVO_DEBUG)
        fprintf(stderr, "Servo %d: Servo position with ID %d not found.\n", get_servo_num(this->pin), id);
}

void Servo180::set_next_pos() {
    if (servo_pos_list.empty()) {
        if (SERVO_DEBUG)
            fprintf(stderr, "Servo %d: Servo position list is empty. Cannot set next position.\n", get_servo_num(this->pin));
        return;
    }

    // Find the next position in the list
    for (size_t i = 0; i < servo_pos_list.size(); ++i) {
        if (servo_pos_list[i].pos > current_pos) {
            this->set_target_pos(servo_pos_list[i].pos);
            return;
        }
    }

    // If at the end, wrap around to the first position
    this->set_target_pos(servo_pos_list[0].pos);
}

void Servo180::set_previous_pos() {
    if (servo_pos_list.empty()) {
        if (SERVO_DEBUG)
            fprintf(stderr, "Servo %d: Servo position list is empty. Cannot set previous position.\n", get_servo_num(this->pin));
        return;
    }

    // Find the previous position in the list
    for (int i = servo_pos_list.size() - 1; i >= 0; --i) {
        if (servo_pos_list[i].pos < current_pos) {
            this->set_target_pos(servo_pos_list[i].pos);
            return;
        }
    }

    // If at the beginning, wrap around to the last position
    this->set_target_pos(servo_pos_list.back().pos);
}

void Servo180::process_commands_list(ESP_Button* previous_button, ESP_Button* next_button) {
    if (next_button->state == 1 && !button_pressed) {
        this->set_next_pos();
        button_pressed = true;
    }
    else if (previous_button->state == 1 && !button_pressed) {
        this->set_previous_pos();
        button_pressed = true;
    }
    this->update_pos();

    // Update button_pressed when buttons are released
    if (next_button->state == 0 && previous_button->state == 0)
        button_pressed = false;
}

void Servo180::process_commands_list(ESP_Button* next_button) {
    if (next_button->state == 1 && !button_pressed) {
        this->set_next_pos();
        button_pressed = true;
    }
    this->update_pos();

    // Update button_pressed when buttons are released
    if (next_button->state == 0)
        button_pressed = false;
}

void Servo180::set_step_size(float step) {
    this->step_size = step;
}

void Servo180::set_inverted(bool inv) {
    this->inverted = inv;
}

// ======================================================================
// GETTERS
// ======================================================================

int Servo180::get_min_pos() {
    return this->min_pos;
}

int Servo180::get_max_pos() {
    return this->max_pos;
}

bool Servo180::is_inverted() {
    return this->inverted;
}

int Servo180::get_default_pos() {
    return this->default_pos;
}