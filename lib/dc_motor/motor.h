// motor.h
// Created on: 2025-10-28
// Author: Sebastien Cabana
// Description: DC motor control interface for an L298N (or similar) H-bridge driver.
//              IN1/IN2 are digital direction pins; EN is a PWM speed pin.

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "pins.h"
#include "debug_options.h"

// A joystick command in polar form.
//   magnitude ∈ [0, 1]  : 0 = stopped, 1 = full speed
//   angle     ∈ (-π, π] : 0 = forward, ±π/2 = turn, ±π = reverse
struct DriveCommand {
    float magnitude;
    float angle;
};

// Tunable constants for the motor subsystem.
struct MotorConfig {
    MotorConfig() = delete;

    static constexpr int   DEFAULT_BASE_SPEED   = 200;
    static constexpr int   DEFAULT_MIN_SPEED    = 150;
    static constexpr float DEFAULT_LEFT_FACTOR  = 1.0f;
    static constexpr float DEFAULT_RIGHT_FACTOR = 1.0f;

    static constexpr int   PWM_MAX_VALUE = 255;

    static constexpr int   MIN_VALID_SPEED  = 0;
    static constexpr int   MAX_VALID_SPEED  = 255;
    static constexpr float MIN_VALID_FACTOR = 0.1f;
    static constexpr float MAX_VALID_FACTOR = 2.0f;
};

// One DC motor driven by an L298N-style H-bridge.
class DCMotor {
public:
    DCMotor(int in1Pin, int in2Pin, int enPin);

    void begin();
    void setSpeed(int speed);
    void stop();

private:
    int  in1Pin_;
    int  in2Pin_;
    int  enPin_;
    bool initialized_;
};

// Coordinates left + right DC motors for a differential drive base.
class MotorController {
public:
    MotorController(DCMotor& left, DCMotor& right);

    void configure(int   baseSpeed       = MotorConfig::DEFAULT_BASE_SPEED,
                   int   minSpeed        = MotorConfig::DEFAULT_MIN_SPEED,
                   float leftCorrection  = MotorConfig::DEFAULT_LEFT_FACTOR,
                   float rightCorrection = MotorConfig::DEFAULT_RIGHT_FACTOR);

    void begin();
    void drive(const DriveCommand& command);
    void stop();

    int getLeftSpeed()  const { return leftSpeed_; }
    int getRightSpeed() const { return rightSpeed_; }

private:
    DCMotor& left_;
    DCMotor& right_;

    int   baseSpeed_;
    int   minSpeed_;
    float leftCorrection_;
    float rightCorrection_;
    int   leftSpeed_;
    int   rightSpeed_;
    bool  initialized_;

    static int   clampSpeed(int speed);
    static float clampCorrection(float factor);

    void computeWheelSpeeds(const DriveCommand& cmd, int& left, int& right) const;
    int  applyCorrectionAndClamp(int speed, float correction) const;
    int  applyMinimumSpeed(int speed) const;
};

#endif // MOTOR_H