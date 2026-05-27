/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@arduino.cc>
 * Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPI.h"

SPIClass SPI;

/**
  * @brief  Default constructor. Uses pin configuration of variant.h.
  *         The peripheral instance is left null; spi_init resolves it from
  *         the variant's PIN_SPI_* defines, which are unambiguous on every
  *         current target.
  */
SPIClass::SPIClass()
{
  _spi.pin_miso = PIN_SPI_MISO.toPinName();
  _spi.pin_mosi = PIN_SPI_MOSI.toPinName();
  _spi.pin_sclk = PIN_SPI_SCK.toPinName();
  _spi.pin_ssel = NC;
}

/**
  * @brief  Peripheral-aware constructor: caller names the SPI instance.
  * @param  instance: SPI_TypeDef* (SPI1, SPI2, SPI3, ...). spi_init routes
  *         each pin's GPIO config via pinmap_pinout_for_peripheral against
  *         this instance, so multi-mapping pins resolve unambiguously.
  * @param  mosi/miso/sclk: SPI data/clock pins. Must all reach `instance`
  *         in PinMap_SPI_MOSI/MISO/SCLK under some AF; otherwise spi_init
  *         errors out via core_debug.
  * @param  ssel: optional hardware-NSS pin (NC_PIN to disable). Must reach
  *         `instance` in PinMap_SPI_SSEL when supplied.
  */
SPIClass::SPIClass(SPI_TypeDef *instance, Pin mosi, Pin miso, Pin sclk,
                   Pin ssel)
{
  _spi.spi      = instance;
  _spi.pin_miso = miso.toPinName();
  _spi.pin_mosi = mosi.toPinName();
  _spi.pin_sclk = sclk.toPinName();
  _spi.pin_ssel = ssel.toPinName();
}

/**
  * @brief  Initialize the SPI instance.
  */
void SPIClass::begin(void)
{
  _spi.handle.State = HAL_SPI_STATE_RESET;
  _spiSettings = SPISettings();
  spi_init(&_spi, _spiSettings.clockFreq,
           _spiSettings.dataMode,
           _spiSettings.bitOrder);
}

/**
  * @brief  Initialize the SPI instance with supplied settings.
  */
void SPIClass::begin(SPISettings settings)
{
  _spi.handle.State = HAL_SPI_STATE_RESET;
  _spiSettings = settings;
  spi_init(&_spi, _spiSettings.clockFreq,
           _spiSettings.dataMode,
           _spiSettings.bitOrder);
}

/**
  * @brief  This function should be used to configure the SPI instance in case you
  *         don't use the default parameters set by the begin() function.
  * @param  settings: SPI settings(clock speed, bit order, data mode).
  */
void SPIClass::beginTransaction(SPISettings settings)
{
  if (_spiSettings != settings) {
    _spiSettings = settings;
    spi_init(&_spi, _spiSettings.clockFreq,
             _spiSettings.dataMode,
             _spiSettings.bitOrder);
  }
}

/**
  * @brief  End the transaction after beginTransaction usage
  */
void SPIClass::endTransaction(void)
{

}

/**
  * @brief  Deinitialize the SPI instance and stop it.
  */
void SPIClass::end(void)
{
  spi_deinit(&_spi);
}

/**
  * @brief  Deprecated function.
  *         Configure the bit order: MSB first or LSB first.
  * @param  bitOrder: MSBFIRST or LSBFIRST
  */
void SPIClass::setBitOrder(BitOrder bitOrder)
{
  _spiSettings.bitOrder = bitOrder;

  spi_init(&_spi, _spiSettings.clockFreq,
           _spiSettings.dataMode,
           _spiSettings.bitOrder);
}

/**
  * @brief  Deprecated function.
  *         Configure the data mode (clock polarity and clock phase)
  * @param  mode: SPI_MODE0, SPI_MODE1, SPI_MODE2 or SPI_MODE3
  */
void SPIClass::setDataMode(uint8_t mode)
{
  setDataMode((SPIMode)mode);
}

void SPIClass::setDataMode(SPIMode mode)
{
  _spiSettings.dataMode = mode;
  spi_init(&_spi, _spiSettings.clockFreq,
           _spiSettings.dataMode,
           _spiSettings.bitOrder);
}

/**
  * @brief  Deprecated function.
  *         Configure the clock speed
  * @param  divider: the SPI clock can be divided by values from 1 to 255.
  *         If 0, default SPI speed is used.
  */
void SPIClass::setClockDivider(uint8_t divider)
{
  if (divider == 0) {
    _spiSettings.clockFreq = SPI_SPEED_CLOCK_DEFAULT;
  } else {
    /* Get clk freq of the SPI instance and compute it */
    _spiSettings.clockFreq = spi_getClkFreq(&_spi) / divider;
  }

  spi_init(&_spi, _spiSettings.clockFreq,
           _spiSettings.dataMode,
           _spiSettings.bitOrder);
}

/**
  * @brief  Transfer one byte on the SPI bus.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  data: byte to send.
  * @param  skipReceive: skip receiving data after transmit or not.
  *         SPI_TRANSMITRECEIVE or SPI_TRANSMITONLY.
  *         Optional, default: SPI_TRANSMITRECEIVE.
  * @return byte received from the slave.
  */
uint8_t SPIClass::transfer(uint8_t data, bool skipReceive)
{
  spi_transfer(&_spi, &data, (!skipReceive) ? &data : NULL, sizeof(uint8_t));
  return data;
}

/**
  * @brief  Transfer two bytes on the SPI bus in 16 bits format.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  data: bytes to send.
  * @param  skipReceive: skip receiving data after transmit or not.
  *         SPI_TRANSMITRECEIVE or SPI_TRANSMITONLY.
  *         Optional, default: SPI_TRANSMITRECEIVE.
  * @return bytes received from the slave in 16 bits format.
  */
uint16_t SPIClass::transfer16(uint16_t data, bool skipReceive)
{
  uint16_t tmp;

  if (_spiSettings.bitOrder) {
    tmp = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
    data = tmp;
  }
  spi_transfer(&_spi, (uint8_t *)&data, (!skipReceive) ? (uint8_t *)&data : NULL, sizeof(uint16_t));

  if (_spiSettings.bitOrder) {
    tmp = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
    data = tmp;
  }

  return data;
}

/**
  * @brief  Transfer several bytes. Only one buffer used to send and receive data.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  buf: pointer to the bytes to send. The bytes received are copy in
  *         this buffer.
  * @param  count: number of bytes to send/receive.
  * @param  skipReceive: skip receiving data after transmit or not.
  *         SPI_TRANSMITRECEIVE or SPI_TRANSMITONLY.
  *         Optional, default: SPI_TRANSMITRECEIVE.
  */
void SPIClass::transfer(void *buf, size_t count, bool skipReceive)
{
  spi_transfer(&_spi, (uint8_t *)buf, (!skipReceive) ? (uint8_t *)buf : NULL, count);

}

/**
  * @brief  Transfer several bytes. One constant buffer used to send and
  *         one to receive data.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  tx_buf: array of Tx bytes that is filled by the user before starting
  *                 the SPI transfer. If NULL, default dummy 0xFF bytes will be
  *                 clocked out.
  * @param  rx_buf: array of Rx bytes that will be filled by the slave during
  *                 the SPI transfer. If NULL, the received data will be discarded.
  * @param  count: number of bytes to send/receive.
  */
void SPIClass::transfer(const void *tx_buf, void *rx_buf, size_t count)
{
  spi_transfer(&_spi, ((const uint8_t *)tx_buf), ((uint8_t *)rx_buf), count);
}


/**
  * @brief  Not implemented.
  */
void SPIClass::usingInterrupt(int interruptNumber)
{
  UNUSED(interruptNumber);
}

/**
  * @brief  Not implemented.
  */
void SPIClass::notUsingInterrupt(int interruptNumber)
{
  UNUSED(interruptNumber);
}

/**
  * @brief  Not implemented.
  */
void SPIClass::attachInterrupt(void)
{
  // Should be enableInterrupt()
}

/**
  * @brief  Not implemented.
  */
void SPIClass::detachInterrupt(void)
{
  // Should be disableInterrupt()
}
