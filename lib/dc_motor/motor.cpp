// motor.cpp
// Created on: 2025-10-28
// Author: Sebastien Cabana
// Description: Implementation of DC motor control for an L298N (or similar)
//              H-bridge driver, including a differential drive controller.

#include <motor.h>
#include <math.h>

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

void MotorController::drive(const DriveCommand& command) {
    if (!initialized_) {
        stop();
        return;
    }

    int rawLeft = 0, rawRight = 0;
    computeWheelSpeeds(command, rawLeft, rawRight);

    leftSpeed_  = applyMinimumSpeed(applyCorrectionAndClamp(rawLeft,  leftCorrection_));
    rightSpeed_ = applyMinimumSpeed(applyCorrectionAndClamp(rawRight, rightCorrection_));

    if (MOTOR_DEBUG) {
        fprintf(stderr, "[Motor] r=%.3f th=%.3f | L=%d R=%d\n",
                command.magnitude, command.angle, leftSpeed_, rightSpeed_);
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

// Differential drive mixing:
//   forward = magnitude * cos(angle)
//   turn    = magnitude * sin(angle)
//   left    = forward + turn
//   right   = forward - turn
void MotorController::computeWheelSpeeds(const DriveCommand& cmd,
                                         int& left, int& right) const {
    float m = cmd.magnitude;
    if (m < 0.0f) m = 0.0f;
    if (m > 1.0f) m = 1.0f;

    const float forward = m * cosf(cmd.angle);
    const float turn    = m * sinf(cmd.angle);

    const float leftNorm  = forward + turn;
    const float rightNorm = forward - turn;

    left  = static_cast<int>(leftNorm  * baseSpeed_);
    right = static_cast<int>(rightNorm * baseSpeed_);
}

int MotorController::clampSpeed(int speed) {
    if (speed < MotorConfig::MIN_VALID_SPEED) return MotorConfig::MIN_VALID_SPEED;
    if (speed > MotorConfig::MAX_VALID_SPEED) return MotorConfig::MAX_VALID_SPEED;
    return speed;
}

float MotorController::clampCorrection(float factor) {
    if (factor < MotorConfig::MIN_VALID_FACTOR) return MotorConfig::MIN_VALID_FACTOR;
    if (factor > MotorConfig::MAX_VALID_FACTOR) return MotorConfig::MAX_VALID_FACTOR;
    return factor;
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