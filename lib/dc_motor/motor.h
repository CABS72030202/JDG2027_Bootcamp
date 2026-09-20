// motor.h
// Created on: 2026-09-20
// Author: Sebastien Cabana
// Description: DC motor control interface for an L298N (or similar) H-bridge driver.
//              IN1/IN2 are digital direction pins; EN is a PWM speed pin.

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "pins.h"
#include "debug_options.h"

// Tunable constants for the motor subsystem.
struct MotorConfig {
    static constexpr int MAX_SPEED_LEVEL = 3;
    static constexpr int SPEED_MULTIPLIERS[MAX_SPEED_LEVEL] = {60, 80, 100};

    static constexpr int   DEFAULT_BASE_SPEED   = 200;
    static constexpr int   DEFAULT_MIN_SPEED    = 150;
    static constexpr float DEFAULT_LEFT_FACTOR  = 1.0f;
    static constexpr float DEFAULT_RIGHT_FACTOR = 1.0f;

    static constexpr int   PWM_MAX_VALUE    = 255;
    static constexpr float TURN_SPEED_RATIO = 0.5f;

    static constexpr int   MIN_VALID_SPEED  = 0;
    static constexpr int   MAX_VALID_SPEED  = 255;
    static constexpr float MIN_VALID_FACTOR = 0.1f;
    static constexpr float MAX_VALID_FACTOR = 2.0f;
};

enum class Direction {
    STOP        = 0,
    NORTH       = 1,
    NORTH_EAST  = 2,
    EAST        = 3,
    SOUTH_EAST  = 4,
    SOUTH       = 5,
    SOUTH_WEST  = 6,
    WEST        = 7,
    NORTH_WEST  = 8
};

// One DC motor driven by an L298N-style H-bridge.
// IN1/IN2 set direction (digital); EN sets speed (PWM).
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
    void drive(Direction direction, int speedLevel);
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
    static int   clampSpeedLevel(int level);
    static float clampCorrection(float factor);

    void computeWheelSpeeds(Direction direction, int& left, int& right) const;
    int  applyCorrectionAndClamp(int speed, float correction) const;
    int  applyMinimumSpeed(int speed) const;
};

#endif // MOTOR_H