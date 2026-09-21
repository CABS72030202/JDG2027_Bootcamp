# JDG2027 Bootcamp

Firmware for a small differential-drive robot controlled over Wi-Fi via
RemoteXY. Built for the ESP32 DevKitC v4 with PlatformIO.

This project is a teaching example: it shows how to structure an embedded
C++ codebase into small, focused libraries, and how to mix discrete and
continuous control representations without breaking existing consumers.

## Hardware

- ESP32 DevKitC v4 (see `doc/esp32_devkitC_v4_pinlayout.png` for pinout)
- L298N or similar H-bridge driver
- Two DC gear motors (left and right wheels)
- Two servos (SG90 or similar) — optional, currently driven by buttons
- A virtual joystick in the RemoteXY phone app

### Motor wiring

Each motor uses three ESP32 pins: two digital outputs for direction (IN1,
IN2) and one PWM output for speed (EN).

    ESP32                 L298N
    -----------------------------------------
    MOTOR_A_IN1_PIN ----> IN1  (left motor)
    MOTOR_A_IN2_PIN ----> IN2
    MOTOR_A_EN_PIN  ----> ENA  (PWM)

    MOTOR_B_IN1_PIN ----> IN3  (right motor)
    MOTOR_B_IN2_PIN ----> IN4
    MOTOR_B_EN_PIN  ----> ENB  (PWM)

Exact pin numbers live in `inc/pins.h`.

## Repository Layout

    JDG2027-Bootcamp/
    |-- README.md
    |-- doc/                  datasheets and reference material
    |-- inc/                  shared headers used by all libraries
    |   |-- debug_options.h   feature flags (MOTOR_DEBUG, SERVO_DEBUG, ...)
    |   `-- pins.h            pin assignments
    |-- lib/                  hardware and protocol libraries
    |   |-- dc_motor/         motor control (this project's core)
    |   |-- esp_controls/     shared control state (buttons, joystick)
    |   |-- remote_xy/        RemoteXY integration (Wi-Fi + GUI)
    |   `-- servo/            servo control (position and continuous)
    |-- platformio.ini        PlatformIO build configuration
    `-- src/
        `-- bootcamp.cpp      main sketch (setup + loop)

Each folder under `lib/` is a self-contained PlatformIO library with its
own `.h` and `.cpp` pair. Nothing outside that folder needs to know how
it works internally.

## Libraries at a Glance

### dc_motor

Controls a differential-drive base made of two DC motors driven by an
L298N-style H-bridge.

`DCMotor` represents one physical motor. It owns its three pins and
exposes `setSpeed(int)` with a signed speed (-255 to +255) and `stop()`.

`MotorController` coordinates the two motors. It takes a `DriveCommand`
struct containing a magnitude and an angle, and converts it to left and
right wheel speeds using the standard differential-drive mixing formula:

    forward = magnitude * cos(angle)
    turn    = magnitude * sin(angle)
    left    = forward + turn
    right   = forward - turn

Every wheel speed is scaled by `baseSpeed`, multiplied by a per-side
correction factor, clamped to +/-255, and raised to `minSpeed` if it
would otherwise stall the motor.

`DriveCommand` is defined in this library's header and is the only thing
the rest of the project needs to know about motor input.

### esp_controls

Central state for everything the user can interact with.

`ESP_Button` represents one button. It handles both momentary and toggle
modes, with debouncing.

`ESP_Axis` represents one analog axis (currently the joystick). It
carries two representations of the same input:

- Discrete: `zone` and `direction` (an `ESP_Direction` enum with 8
  compass values plus `ESP_NONE`).
- Continuous: `magnitude` (0 to 1) and `angle` (-π to π, 0 = forward).

Both are kept in sync every frame. Consumers pick whichever they need.
See "Two Joystick Representations" below.

### remote_xy

Wraps the RemoteXY library.

It starts a Wi-Fi SoftAP that the phone app connects to, reads joystick
and button values from the RemoteXY GUI, and pushes them into
`esp_controls`. It also runs a custom `CRemoteXYNet_WiFiPoint_APSTA` to
keep the Wi-Fi channel aligned with ESP-NOW, which is used elsewhere in
other JDG projects.

Call `xy_init()` once in `setup()` and `xy_handle_remotexy()` every
iteration of `loop()`.

### servo

Servo control for two types of servos.

`Servo180` is a positional servo. It takes a min and max range, a
default position, and a step size. It supports direct targeting via
`set_target_pos`, stepwise commands from buttons or a joystick, and a
"position list" for preset navigation.

`ServoContinuous` is a continuous-rotation servo. It exposes
`rotate_cw()`, `rotate_ccw()`, `stop()`, and an optional timeout that
auto-stops rotation after a configured duration.

Note: the servo library still consumes `ESP_Direction` values from
`esp_controls`. That enum is kept around specifically for this reason.
The joystick is not wired to the servos in the current sketch, but the
capability is there.

## Two Joystick Representations

The joystick is the only user input with two valid interpretations, and
this project deliberately supports both.

    Representation   Where it lives                        Who uses it        Angle 0 means
    ----------------------------------------------------------------------------------------
    Discrete         ESP_Axis::direction, ESP_Axis::zone   Servo library      East (right)
    Continuous       ESP_Axis::magnitude, ESP_Axis::angle  Motor controller   Forward

These are not the same coordinate system, and that is intentional.

The discrete form follows the compass convention, where 0 degrees is
East and angles grow counter-clockwise. It is convenient for "which way
is the stick pointing?" checks.

The continuous form follows the robot heading convention, where 0
degrees is forward and angles grow clockwise. It is the natural form
for differential-drive mixing.

Both are produced from the same raw `(x, y)` in `update_xy_data()` in
`esp_xy.cpp`. If you are ever confused about why a 90-degree value
behaves differently in two places, this is why. The debug print from
`esp_print_states()` shows both representations side by side.

## Building and Flashing

Requires PlatformIO, either the CLI or the VS Code extension.

    pio run                    # build
    pio run --target upload    # flash the board
    pio device monitor         # serial monitor

Default serial speed is set in `platformio.ini`.

## Running the Robot

1. Power the ESP32.
2. On your phone, connect to the Wi-Fi network `ITR`. The password is
   in `lib/remote_xy/esp_xy.h`.
3. Open the RemoteXY app and connect to the Wi-Fi network `ITR`.
4. Use the virtual joystick to drive the robot:
   - Push up: forward
   - Push down: reverse
   - Push left: spin counter-clockwise
   - Push right: spin clockwise
   - Diagonal: curve in that direction
5. The four buttons are wired to the two servos in `bootcamp.cpp`.

## Debug Options

Set these flags in `inc/debug_options.h`.

    Flag                  Effect
    --------------------------------------------------------------------------
    MOTOR_DEBUG           Prints polar command and wheel speeds each update
    SERVO_DEBUG           Prints servo init, target changes, rotation events
    ESP_PRINT_CONTROLS    Prints button states and both joystick representations

Turn them on individually to avoid drowning in output.

## Tuning

Most tunable values live in one place: `MotorConfig` in
`lib/dc_motor/motor.h`.

    Constant                                   Default    Meaning
    ------------------------------------------------------------------------
    DEFAULT_BASE_SPEED                         200        Max wheel speed (PWM)
    DEFAULT_MIN_SPEED                          150        Min wheel speed before stall
    DEFAULT_LEFT_FACTOR                        1.0        Left side correction factor
    DEFAULT_RIGHT_FACTOR                       1.0        Right side correction factor
    PWM_MAX_VALUE                              255        Hardware limit
    MIN_VALID_FACTOR / MAX_VALID_FACTOR        0.1 / 2.0  Clamp range for corrections

The bootcamp sketch overrides these at startup via `robot.configure()`.

If the robot drifts to one side, adjust the correction factors. If a
wheel spins the wrong way when both should go forward, swap that
motor's two IN pins. Direction is a wiring fact, not a code problem.

## Extending the Project

Adding a new motor-side feature: open `lib/dc_motor/motor.h`. Everything
the rest of the project needs (`DCMotor`, `MotorController`,
`DriveCommand`, `MotorConfig`) is there. If you change the shape of
`DriveCommand`, only `esp_xy.cpp` needs updating, because it is the sole
producer of that type for now.

Adding a new input: add fields to `ESP_Axis` or `ESP_Button` in
`esp_controls.h`, populate them in `update_xy_data()` in `esp_xy.cpp`,
and consume them wherever needed. Nothing else has to change.

Adding a new library: create `lib/<name>/<name>.h` and
`lib/<name>/<name>.cpp`. PlatformIO picks it up automatically on the
next build. Include it from `src/bootcamp.cpp` with
`#include "<name>.h"`.

## Known Quirks

`esp_xy.h` includes `motor.h` only to get the `DriveCommand` struct.
This is a small layering wart. If it ever becomes a problem, move
`DriveCommand` into a neutral header, or return it from
`xy_get_drive_command()` via out-parameters.

`ESP_Direction` is still present even though nothing in the motor path
uses it. This is intentional: the servo library depends on it. Removing
it would require refactoring `servo.h` first.

`esp_controls.cpp` uses `static` storage for buttons and the joystick,
while `servo.h` and `servo.cpp` hold per-instance state. This split is
a historical artifact and works fine, but if you ever add a second
joystick, `esp_controls` will need a redesign.