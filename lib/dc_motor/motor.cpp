// motor.cpp
// Created on: 2026-09-20
// Author: Sebastien Cabana
// Description: Implementation of DC motor control for an L298N (or similar)
//              H-bridge driver, including a differential drive controller.

#include <motor.h>

constexpr int MotorConfig::SPEED_MULTIPLIERS[MotorConfig::MAX_SPEED_LEVEL];

// ============================================================================
// DCMotor
// ============================================================================

DCMotor::DCMotor(int in1Pin, int in2Pin, int enPin)
    : in1Pin_(in1Pin), in2Pin_(in2Pin), enPin_(enPin), initialized_(false) {}

void DCMotor::begin() {
    pinMode(in1Pin_, OUTPUT);
    pinMode(in2Pin_, OUTPUT);
    pinMode(enPin_,  OUTPUT);
    stop();
    initialized_ = true;
}

void DCMotor::setSpeed(int speed) {
    if (!initialized_) return;

    if (speed >  MotorConfig::PWM_MAX_VALUE) speed =  MotorConfig::PWM_MAX_VALUE;
    if (speed < -MotorConfig::PWM_MAX_VALUE) speed = -MotorConfig::PWM_MAX_VALUE;

    digitalWrite(in1Pin_, speed > 0 ? HIGH : LOW);
    digitalWrite(in2Pin_, speed > 0 ? LOW  : HIGH);
    analogWrite(enPin_, abs(speed));
}

void DCMotor::stop() {
    digitalWrite(in1Pin_, LOW);
    digitalWrite(in2Pin_, LOW);
    analogWrite(enPin_, 0);
}

// ============================================================================
// MotorController
// ============================================================================

MotorController::MotorController(DCMotor& left, DCMotor& right)
    : left_(left), right_(right),
      baseSpeed_(MotorConfig::DEFAULT_BASE_SPEED),
      minSpeed_(MotorConfig::DEFAULT_MIN_SPEED),
      leftCorrection_(MotorConfig::DEFAULT_LEFT_FACTOR),
      rightCorrection_(MotorConfig::DEFAULT_RIGHT_FACTOR),
      leftSpeed_(0), rightSpeed_(0), initialized_(false) {}

void MotorController::configure(int baseSpeed, int minSpeed,
                                float leftCorrection, float rightCorrection) {
    baseSpeed_       = clampSpeed(baseSpeed);
    minSpeed_        = clampSpeed(minSpeed);
    leftCorrection_  = clampCorrection(leftCorrection);
    rightCorrection_ = clampCorrection(rightCorrection);

    if (minSpeed_ > baseSpeed_) minSpeed_ = baseSpeed_;
}

void MotorController::begin() {
    left_.begin();
    right_.begin();
    stop();
    initialized_ = true;
}

void MotorController::drive(Direction direction, int speedLevel) {
    if (!initialized_) {
        stop();
        return;
    }

    speedLevel = clampSpeedLevel(speedLevel);

    int rawLeft = 0, rawRight = 0;
    computeWheelSpeeds(direction, rawLeft, rawRight);

    const int percent = MotorConfig::SPEED_MULTIPLIERS[speedLevel - 1];
    rawLeft  = (rawLeft  * percent) / 100;
    rawRight = (rawRight * percent) / 100;

    leftSpeed_  = applyMinimumSpeed(applyCorrectionAndClamp(rawLeft,  leftCorrection_));
    rightSpeed_ = applyMinimumSpeed(applyCorrectionAndClamp(rawRight, rightCorrection_));

    if (MOTOR_DEBUG) {
        fprintf(stderr, "[Motor] Dir=%d Lvl=%d | L=%d R=%d\n",
                static_cast<int>(direction), speedLevel,
                leftSpeed_, rightSpeed_);
    }

    left_.setSpeed(leftSpeed_);
    right_.setSpeed(rightSpeed_);
}

void MotorController::stop() {
    left_.stop();
    right_.stop();
    leftSpeed_  = 0;
    rightSpeed_ = 0;
}

int MotorController::clampSpeed(int speed) {
    if (speed < MotorConfig::MIN_VALID_SPEED) return MotorConfig::MIN_VALID_SPEED;
    if (speed > MotorConfig::MAX_VALID_SPEED) return MotorConfig::MAX_VALID_SPEED;
    return speed;
}

int MotorController::clampSpeedLevel(int level) {
    if (level < 1) return 1;
    if (level > MotorConfig::MAX_SPEED_LEVEL) return MotorConfig::MAX_SPEED_LEVEL;
    return level;
}

float MotorController::clampCorrection(float factor) {
    if (factor < MotorConfig::MIN_VALID_FACTOR) return MotorConfig::MIN_VALID_FACTOR;
    if (factor > MotorConfig::MAX_VALID_FACTOR) return MotorConfig::MAX_VALID_FACTOR;
    return factor;
}

void MotorController::computeWheelSpeeds(Direction direction,
                                         int& left, int& right) const {
    const int base   = baseSpeed_;
    const int turnIn = static_cast<int>(base * MotorConfig::TURN_SPEED_RATIO);

    switch (direction) {
        case Direction::NORTH:       left =  base;    right =  base;    break;
        case Direction::NORTH_EAST:  left =  turnIn;  right =  base;    break;
        case Direction::NORTH_WEST:  left =  base;    right =  turnIn;  break;
        case Direction::SOUTH:       left = -base;    right = -base;    break;
        case Direction::SOUTH_EAST:  left = -turnIn;  right = -base;    break;
        case Direction::SOUTH_WEST:  left = -base;    right = -turnIn;  break;
        case Direction::EAST:        left = -base;    right =  base;    break;
        case Direction::WEST:        left =  base;    right = -base;    break;
        case Direction::STOP:
        default:                     left =  0;       right =  0;       break;
    }
}

int MotorController::applyCorrectionAndClamp(int speed, float correction) const {
    int corrected = static_cast<int>(speed * correction);

    if (corrected >  MotorConfig::PWM_MAX_VALUE) corrected =  MotorConfig::PWM_MAX_VALUE;
    if (corrected < -MotorConfig::PWM_MAX_VALUE) corrected = -MotorConfig::PWM_MAX_VALUE;

    return corrected;
}

int MotorController::applyMinimumSpeed(int speed) const {
    if (speed == 0) return 0;

    if (speed > 0 && speed < minSpeed_)  return  minSpeed_;
    if (speed < 0 && speed > -minSpeed_) return -minSpeed_;

    return speed;
}