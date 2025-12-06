/**
 * Dps3xxDma.h - DMA-enhanced DPS3xx driver for non-blocking I2C reads
 *
 * Extends Infineon's arduino-xensiv-dps3xx library to support:
 *   - Non-blocking I2C DMA transfers via Wire DMA API
 *   - Non-FIFO PRS_RDY polling (iNav-style continuous measurement)
 *
 * Interface inspired by INav project's barometer integration.
 * Class hierarchy: DpsClass -> Dps3xx -> Dps3xxDma
 */

#ifndef DPS3XXDMA_H_INCLUDED
#define DPS3XXDMA_H_INCLUDED

#include "Dps3xx.h"
#include <Wire.h>

class Dps3xxDma : public Dps3xx
{
public:
    /**
     * @brief Override base begin() to reset DMA state flag.
     * @param bus          I2C bus instance.
     * @param slaveAddress I2C address of DPS3xx (0x76 or 0x77).
     * @return true on successful sensor init, false otherwise.
     */
    bool begin(TwoWire &bus, uint8_t slaveAddress);

    /**
     * @brief Configure DPS for pressure-only, 32 Hz, OSR x16, FIFO disabled.
     *        Follows INav-style startup sequence.
     * @return DPS__SUCCEEDED or error code on failure.
     */
    int16_t startPressure32HzNoFIFO();

    /**
     * @brief Starts continuous pressure measurement with specified rates.
     *
     * @param measureRate      DPS__MEASUREMENT_RATE_1, _2, ... _128
     * @param oversamplingRate DPS__OVERSAMPLING_RATE_1, _2, ... _128
     * @return DPS__SUCCEEDED or error code on failure.
     */
    int16_t startMeasurePressureContNoFIFO(uint8_t measureRate, uint8_t oversamplingRate);

    /* ---------- DMA-enhanced helpers ---------- */

    /**
     * @brief Initiate a non-blocking DMA read of a register block.
     * @param regBlock  Register address + length to fetch.
     * @param dma_buf   User buffer for DMA to fill (must be WIRE_DMA_BUFFER on H7).
     * @return DPS__SUCCEEDED or error code if busy/invalid.
     */
    int16_t readBlockDMA(RegBlock_t regBlock, uint8_t* dma_buf);

    /**
     * @brief Query whether the previous DMA read has completed.
     * @return true if no DMA in progress or transfer finished.
     */
    bool isReadDone();

    /**
     * @brief Poll the sensor's PRS_RDY bit for new pressure data.
     * @return true if a fresh pressure sample is available.
     */
    bool pressureSampleIsReady();

    /**
     * @brief Poll the sensor's TEMP_RDY bit for new temperature data.
     * @return true if a fresh temperature sample is available.
     */
    bool tempSampleIsReady();

    /**
     * @brief Convert raw 24-bit pressure sample to Pascals.
     * @param raw  Signed 24-bit sample (sign-extended to int32_t).
     * @return Floating-point pressure in Pa.
     */
    float pressurePaFromRaw(int32_t raw) { return calcPressure(raw); }

    /**
     * @brief Convert raw 24-bit temperature sample to Celsius.
     * @param raw  Signed 24-bit sample (sign-extended to int32_t).
     * @return Floating-point temperature in degrees C.
     */
    float tempCFromRaw(int32_t raw) { return calcTemp(raw); }

private:
    /**
     * @brief Indicates if an I2C DMA read is currently in progress.
     *        When true, further DMA reads are rejected until complete.
     */
    volatile bool dmaBusy_ { false };
};

#endif // DPS3XXDMA_H_INCLUDED
