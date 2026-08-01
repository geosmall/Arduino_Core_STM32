// SPDX-License-Identifier: MIT
// Copyright (c) 2026 George Small
// See the LICENSE file in this library's root directory.

#include "BoardAlignment.h"
#include <math.h>

namespace BoardAlignment {

namespace {
constexpr float DEG_TO_RAD_F = 0.01745329251994329577f;
}

Mat3f makeBetaflightBoardAlignMatrix(float yaw_deg,
                                     float pitch_deg,
                                     float roll_deg) {
    // Negative input angles produce Betaflight's left-hand user convention:
    // R = Rz(-yaw) * Ry(-pitch) * Rx(-roll).
    const float yaw   = -yaw_deg   * DEG_TO_RAD_F;
    const float pitch = -pitch_deg * DEG_TO_RAD_F;
    const float roll  = -roll_deg  * DEG_TO_RAD_F;

    const float cy = cosf(yaw),   sy = sinf(yaw);
    const float cp = cosf(pitch), sp = sinf(pitch);
    const float cr = cosf(roll),  sr = sinf(roll);

    Mat3f R;
    R.m[0][0] = cy * cp;
    R.m[0][1] = cy * sp * sr - sy * cr;
    R.m[0][2] = cy * sp * cr + sy * sr;

    R.m[1][0] = sy * cp;
    R.m[1][1] = sy * sp * sr + cy * cr;
    R.m[1][2] = sy * sp * cr - cy * sr;

    R.m[2][0] = -sp;
    R.m[2][1] = cp * sr;
    R.m[2][2] = cp * cr;
    return R;
}

Mat3f makeTargetSensorAlignMatrix(IMUAlignment a) {
    // Each case is the matrix R such that R*(x,y,z) reproduces the vector
    // permutation in Betaflight's alignSensorViaRotation switch.
    Mat3f R = {};
    switch (a) {
    default:
    case IMUAlignment::CW0_DEG:         // (x,y,z) -> ( x,  y,  z)
        R.m[0][0] =  1.0f; R.m[1][1] =  1.0f; R.m[2][2] =  1.0f;
        break;
    case IMUAlignment::CW90_DEG:        // (x,y,z) -> ( y, -x,  z)
        R.m[0][1] =  1.0f; R.m[1][0] = -1.0f; R.m[2][2] =  1.0f;
        break;
    case IMUAlignment::CW180_DEG:       // (x,y,z) -> (-x, -y,  z)
        R.m[0][0] = -1.0f; R.m[1][1] = -1.0f; R.m[2][2] =  1.0f;
        break;
    case IMUAlignment::CW270_DEG:       // (x,y,z) -> (-y,  x,  z)
        R.m[0][1] = -1.0f; R.m[1][0] =  1.0f; R.m[2][2] =  1.0f;
        break;
    case IMUAlignment::CW0_DEG_FLIP:    // (x,y,z) -> (-x,  y, -z)
        R.m[0][0] = -1.0f; R.m[1][1] =  1.0f; R.m[2][2] = -1.0f;
        break;
    case IMUAlignment::CW90_DEG_FLIP:   // (x,y,z) -> ( y,  x, -z)
        R.m[0][1] =  1.0f; R.m[1][0] =  1.0f; R.m[2][2] = -1.0f;
        break;
    case IMUAlignment::CW180_DEG_FLIP:  // (x,y,z) -> ( x, -y, -z)
        R.m[0][0] =  1.0f; R.m[1][1] = -1.0f; R.m[2][2] = -1.0f;
        break;
    case IMUAlignment::CW270_DEG_FLIP:  // (x,y,z) -> (-y, -x, -z)
        R.m[0][1] = -1.0f; R.m[1][0] = -1.0f; R.m[2][2] = -1.0f;
        break;
    }
    return R;
}

Mat3f makeSensorToVehicleMatrix(IMUAlignment chip,
                                float yaw_deg,
                                float pitch_deg,
                                float roll_deg) {
    const Mat3f sensor_to_board  = makeTargetSensorAlignMatrix(chip);
    const Mat3f board_to_vehicle = makeBetaflightBoardAlignMatrix(yaw_deg, pitch_deg, roll_deg);
    // Left operand applies last: v_vehicle = board_to_vehicle * (sensor_to_board * v).
    return mat3_mul_mat3(board_to_vehicle, sensor_to_board);
}

}  // namespace BoardAlignment
