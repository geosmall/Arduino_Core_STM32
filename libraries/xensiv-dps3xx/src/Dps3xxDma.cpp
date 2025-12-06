/**
 * Dps3xxDma.cpp - DMA-enhanced DPS3xx driver implementation
 */

#include "Dps3xxDma.h"
#include "util/dps_config.h"

/* ------------------------------------------------------------------ */
/*      Override base class begin() to return success/fail bool       */
/*      IMPORTANT: Does NOT call bus.begin() to preserve DMA state    */
/* ------------------------------------------------------------------ */
bool Dps3xxDma::begin(TwoWire& bus, uint8_t slaveAddress)
{
    // Reset DMA state
    dmaBusy_ = false;

    // Manually set up I2C connection WITHOUT calling bus.begin()
    // (bus should already be initialized by user, and calling begin()
    // again would reset the I2C peripheral and break DMA)
    m_initFail = 0U;
    m_SpiI2c = 1U;          // Mark as I2C mode
    m_i2cbus = &bus;
    m_slaveAddress = slaveAddress;

    // Skip bus.begin() - user must have already called it!
    // DpsClass::begin() calls m_i2cbus->begin() which resets I2C peripheral
    // and breaks any DMA configuration.

    delay(50);  // DPS3xx startup time

    // Call the sensor initialization (reads coefficients, etc.)
    init();

    // Return true if init() did not set m_initFail
    return (m_initFail == 0U);
}

/* ------------------------------------------------------------------ */
/*      INav-style initialization: 32 Hz, OSR 16x, no FIFO            */
/* ------------------------------------------------------------------ */
int16_t Dps3xxDma::startPressure32HzNoFIFO()
{
    return startMeasurePressureContNoFIFO(DPS__MEASUREMENT_RATE_32, DPS__OVERSAMPLING_RATE_16);
}

int16_t Dps3xxDma::startMeasurePressureContNoFIFO(uint8_t measureRate, uint8_t oversamplingRate)
{
    // Abort if initialization failed
    if (m_initFail) {
        return DPS__FAIL_INIT_FAILED;
    }
    // Abort if device is not in idle mode
    if (m_opMode != dps::Mode::IDLE) {
        return DPS__FAIL_TOOBUSY;
    }
    // Abort if speed and precision are too high
    if (calcBusyTime(measureRate, oversamplingRate) >= DPS3xx__MAX_BUSYTIME) {
        return DPS__FAIL_UNFINISHED;
    }
    // Update precision and measuring rate
    if (configPressure(measureRate, oversamplingRate)) {
        return DPS__FAIL_UNKNOWN;
    }
    // Disable result FIFO
    if (disableFIFO()) {
        return DPS__FAIL_UNKNOWN;
    }
    // Start measuring in background mode (continuous pressure)
    if (setOpMode(dps::Mode::CONT_PRS)) {
        return DPS__FAIL_UNKNOWN;
    }
    return DPS__SUCCEEDED;
}

/* ------------------------------------------------------------------ */
/*      Poll PRS_RDY / TEMP_RDY status bits                           */
/* ------------------------------------------------------------------ */
bool Dps3xxDma::pressureSampleIsReady()
{
    return readByteBitfield(dps::config_registers[dps::PRS_RDY]) == 1;
}

bool Dps3xxDma::tempSampleIsReady()
{
    return readByteBitfield(dps::config_registers[dps::TEMP_RDY]) == 1;
}

/* ------------------------------------------------------------------ */
/*      Non-blocking DMA read, and check isReadDone                   */
/* ------------------------------------------------------------------ */
int16_t Dps3xxDma::readBlockDMA(RegBlock_t regBlock, uint8_t* dma_buf)
{
    if (!m_i2cbus) {
        return -10;  // No bus
    }
    if (!dma_buf) {
        return -11;  // No buffer
    }
    if (regBlock.length == 0) {
        return -12;  // Zero length
    }
    if (dmaBusy_) {
        return -13;  // Busy
    }

    // Set register pointer
    m_i2cbus->beginTransmission(m_slaveAddress);
    m_i2cbus->write(regBlock.regAddress);
    uint8_t endResult = m_i2cbus->endTransmission(false);  // Repeated-START
    if (endResult != 0) {
        return -20 - endResult;  // I2C write failed (-21, -22, etc.)
    }

    // Fire off DMA read into user buffer
    if (!m_i2cbus->requestFromDMA(m_slaveAddress, dma_buf, regBlock.length, true)) {
        return -30;  // DMA request failed
    }

    dmaBusy_ = true;  // Mark sensor DMA RX as in progress
    return DPS__SUCCEEDED;
}

bool Dps3xxDma::isReadDone()
{
    if (!dmaBusy_) {
        return true;  // Nothing running
    }

    if (m_i2cbus && m_i2cbus->dmaTransferDone()) {
        dmaBusy_ = false;  // Mark sensor DMA RX as finished
        return true;
    }
    return false;  // Still in progress
}
