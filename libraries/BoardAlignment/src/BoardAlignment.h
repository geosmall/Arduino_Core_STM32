#pragma once

#include <stdint.h>
#include "targets/config/ConfigTypes.h"  // IMUAlignment

namespace BoardAlignment {

struct Vec3f {
    float x;
    float y;
    float z;
};

struct Mat3f {
    float m[3][3];
};

// v_out = R * v_in  (9 mul, 6 add)
inline Vec3f mat3_mul_vec3(const Mat3f& R, const Vec3f& v) {
    return {
        R.m[0][0] * v.x + R.m[0][1] * v.y + R.m[0][2] * v.z,
        R.m[1][0] * v.x + R.m[1][1] * v.y + R.m[1][2] * v.z,
        R.m[2][0] * v.x + R.m[2][1] * v.y + R.m[2][2] * v.z,
    };
}

// C = A * B  (27 mul, 18 add). Rolled local because CMSIS_DSP's
// arm_mat_mult_f32 needs arm_matrix_instance_f32 setup overhead.
inline Mat3f mat3_mul_mat3(const Mat3f& A, const Mat3f& B) {
    Mat3f C;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            C.m[i][j] = A.m[i][0] * B.m[0][j]
                      + A.m[i][1] * B.m[1][j]
                      + A.m[i][2] * B.m[2][j];
        }
    }
    return C;
}

// User board-to-vehicle alignment in Betaflight's 3-2-1 Euler convention.
// R_align = Rz(-yaw) * Ry(-pitch) * Rx(-roll), with positive user angles
// following a left-hand sense about each named axis (Betaflight semantics).
// Vehicle/body frame: +X forward, +Y left, +Z up (FLU). Input degrees.
Mat3f makeBetaflightBoardAlignMatrix(float yaw_deg,
                                     float pitch_deg,
                                     float roll_deg);

// Chip-to-board sensor alignment. Direct 8-case mapping mirroring
// Betaflight's alignSensorViaRotation() in
// betaflight/src/main/sensors/boardalignment.c:94-145.
Mat3f makeTargetSensorAlignMatrix(IMUAlignment a);

}  // namespace BoardAlignment
