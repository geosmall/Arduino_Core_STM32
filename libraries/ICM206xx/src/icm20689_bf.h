/*
 * ICM20689 Betaflight Driver Header
 *
 * Adapted from Betaflight: drivers/accgyro/accgyro_spi_icm20689.h
 * Copyright (C) Betaflight Contributors
 *
 * This file is part of the ICM20689 Arduino library, derived from Betaflight.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ICM20689_BF_H
#define ICM20689_BF_H

#include "bf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Detection and initialization
uint8_t icm20689SpiDetect(const extDevice_t *dev);
void icm20689InitGyro(gyroDev_t *gyro);
void icm20689InitAcc(accDev_t *acc);

// Data reading
bool icm20689GyroRead(gyroDev_t *gyro);
bool icm20689AccRead(accDev_t *acc);

#ifdef __cplusplus
}
#endif

#endif // ICM20689_BF_H
