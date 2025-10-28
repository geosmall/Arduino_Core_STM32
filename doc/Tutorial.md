# Tutorials.md (Draft Outline)

## Overview

This document provides step-by-step tutorials for configuring, adapting, and testing **dRehmFlight STM32** — a port of Nicholas Rehm’s original Teensy-based VTOL flight controller — for STM32-based boards supported by **Arduino_Core_STM32** (e.g., NOXE V3, Matek H743 Wing V2, Nucleo-F411RE, etc.). Each section corresponds to a core setup or tuning task, updated to reflect STM32 HAL and hardware differences.

## Tutorials

1. **General Instructions for First-Time Setup**

   * Overview of the full workflow from wiring to first hover test.
   * How to verify firmware upload and serial communication.
   * Key safety practices when testing motors or code changes.

2. **Pin Use**

   * Identifying board-specific pinouts from flight controller documentation.
   * Reviewing fixed pin allocations for IMU, UART (IBus/SBUS), and ESC/servo outputs.
   * Updating pin definitions only for compatible alternate boards.

3. **Setting Up Radio Connection**

   * Wiring IBus/SBUS to correct UART port; checking signal inversion if needed.
   * Selecting receiver type in configuration and verifying channel mapping.
   * Monitoring live channel values via Serial Monitor or RTT for calibration.

4. **Verifying Good IMU Data**
   This tutorial ensures the onboard **ICM-42688-P** IMU is functioning properly and correctly oriented before any flight testing. The steps below closely follow the process in the original dRehmFlight documentation, adapted for STM32-based boards.

   **Objective:** Verify that the IMU is communicating, producing valid readings, and properly aligned with the aircraft coordinate frame.

   **Procedure:**

   1. **Confirm Communication:**

      * Power the flight controller via USB or battery and open the Serial Monitor (115200 baud).
      * Upon startup, confirm the message `ICM-42688-P detected` or similar appears. If not, check SPI wiring and chip-select pin configuration.
      * Ensure that the sensor initialization routine in `IMUinit()` executes without errors.

   2. **View Raw Sensor Data:**

      * Within the Arduino IDE, open *Tools → Serial Plotter* to visualize real-time IMU output.
      * Move the board gently and observe that **gyro (deg/sec)** and **accel (g)** traces respond smoothly without spikes or flatlines.
      * Each axis (X, Y, Z) should respond when rotated or moved along that axis.

   3. **Run Bias Calibration:**

      * Ensure the board is perfectly still on a flat surface.
      * In the setup or via serial command, trigger the `calculateIMUerror()` or equivalent calibration function.
      * This routine samples gyro and accelerometer data for several seconds to compute offsets.
      * Once completed, you should see printed bias values near zero for accelerometer (gravity aligned with +Z) and small gyro drift (<0.05 °/s).

   4. **Check Orientation:**

      * Verify IMU axis alignment matches the dRehmFlight coordinate convention:

        * +X forward, +Y left, +Z up.
      * When the board is tilted forward, pitch increases; when tilted right, roll decreases (left-handed system).
      * If any axis appears reversed, update the sign mapping in the IMU data acquisition routine before proceeding.

   5. **Verify Stability:**

      * Observe that small vibrations or taps result in proportional gyro response without saturation.
      * Ensure the noise level at rest remains low and stable (a few LSBs of jitter).
      * If the signal is erratic, confirm solid power and ground connections, and check for EMI or cable strain near the IMU.

   6. **Optional:**

      * Record several seconds of IMU output and compare average bias results over multiple runs to confirm repeatability.
      * Once confirmed stable, save or hardcode the bias offsets if persistent calibration is not implemented.

5. **Mounting the Flight Controller in Your Aircraft** the Flight Controller in Your Aircraft**

   * Aligning IMU axes with aircraft coordinate frame.
   * Using vibration damping mounts or foam pads.
   * Routing cables to minimize strain or interference.

6. **Alternative Actuator Outputs: OneShot125 and Conventional PWM**

   * Switching between OneShot125 and standard PWM modes.
   * Verifying correct PWM frequency and duty range for ESCs/servos.
   * Testing outputs for correct motor numbering and direction.

     * **6.1 Adding Another OneShot125 Controlled Motor**: Expanding motor output configuration, mapping an additional timer channel.
     * **6.2 Adding a PWM Controlled Servo or Motor**: Adding servo variables in controlMixer and validating smooth movement at 50 Hz.

7. **Calibrating ESCs**

   * Running ESC calibration mode to synchronize throttle range.
   * Checking startup tones and throttle endpoints.
   * Confirming motor direction and preventing overcurrent events.

8. **Control Mixing: Basic Fixed Dynamics**

   * Using the default quadrotor mixer as a reference.
   * Modifying roll, pitch, yaw scaling factors for vehicle geometry.
   * Testing stability response while tethered.

9. **Control Mixing: Unstabilized Commands Direct From the Transmitter**

   * Disabling PID loops to pass RC inputs directly to motors.
   * Verifying correct channel orientation (no reversals).
   * Useful for diagnosing mixing or radio input issues.

10. **Control Mixing: Advanced Variable Dynamics**

    * Implementing dual-mode mixing for VTOL or hybrid aircraft.
    * Assigning hover and forward-flight mixes with transition control.
    * Using `floatFaderLinear()` for smooth blending between states.

11. **Basic Fading**

    * Creating a fade variable tied to a radio fader or switch.
    * Mixing hover and forward flight control outputs in one expression.
    * Adjusting fade speed for stable mode transitions.

12. **Controller Selection and Tuning**

    * Choosing between rate, basic angle, or advanced angle controllers.
    * Selecting the active controller in `loop()` and adjusting PID gains.
    * Performing tuning in hover mode before forward flight tests.

      * **12.1 Rate Controller**: Set max roll/pitch/yaw rates; start with P gains, tune D for damping, then add I for drift correction.
      * **12.2 Basic Angle Controller**: Define target angles; tune outer loop P/I terms for gentle stability; maintain yaw on rate control.
      * **12.3 Advanced Angle Controller**: Cascaded inner rate and outer angle loop; adjust B-loop damping; refine for precise attitude hold.

13. **MPU9250 Integration**

    * Legacy IMU reference for Teensy-based systems.
    * Pinout and SPI wiring differences versus ICM-42688-P.
    * Optional backward-compatibility configuration guidance.

14. **Basics of Coding with Arduino**

    * Understanding Arduino structure (`setup()`, `loop()`, and modular includes).
    * Modifying and re-uploading firmware safely.
    * Using serial output for debugging and data inspection.
