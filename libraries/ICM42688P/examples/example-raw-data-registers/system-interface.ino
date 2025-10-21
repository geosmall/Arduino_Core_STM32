/* --------------------------------------------------------------------------------------
 *  Extern functions definition - Invensense to UVOS adapters
 * -------------------------------------------------------------------------------------- */

#include "stm32f4xx_ll_spi.h"

/* Default SPI frequency from BoardConfig */
static uint32_t spi_freq_hz_ = BoardConfig::imu.spi.freq_hz;

#ifdef __cplusplus
extern "C" {
#endif

void inv_board_hal_init(void);
void inv_spi_chip_select_setup_delay(void);
void inv_spi_chip_select_hold_time(void);
void inv_spi_bus_select_device(void);
void inv_spi_bus_deselect_device(void);
uint8_t inv_spi_transfer_byte(uint8_t txByte);
int inv_spi_bus_read_registers(uint8_t addr, uint8_t count, uint8_t* data);
int inv_spi_bus_write_register(uint8_t reg, const uint8_t* data);

/******************************************************/
/* Low-level DWT based delay interface implementation */
/******************************************************/

static uint32_t usTicks_; // Holds DWT ticks per microsecond
static SPI_HandleTypeDef* spi_hdl_; // Used for STM32 HAL/LL SPI access
static PinName cs_pin_; // Hold for fast CS pin access


void DelayUs(uint32_t delay_us);
void DelayNs(uint32_t delay_ns);
spi_status_e TransferLL(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t len);

void DelayUs(uint32_t delay_us)
{
    const uint32_t start  = dwt_getCycles();
    const uint32_t ticks = delay_us * usTicks_;
    while ((dwt_getCycles() - start) < ticks) { }
}

void DelayNs(uint32_t delay_ns)
{
    const uint32_t start  = DWT->CYCCNT;
    const uint32_t ticks = (delay_ns * usTicks_) / 1000;
    while ((DWT->CYCCNT - start) < ticks) { }
}

spi_status_e TransferLL(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t len)
{
  spi_status_e ret = SPI_OK;
  uint32_t tickstart, size = len;
  SPI_TypeDef *_SPI = spi_hdl_->Instance;
  uint8_t *tx_buf = (uint8_t *)tx_buffer;

  if (len == 0) {
    ret = SPI_ERROR;
  } else {
    tickstart = HAL_GetTick();

#if defined(SPI_CR2_TSIZE)
    /* Start transfer */
    LL_SPI_SetTransferSize(_SPI, size);
    LL_SPI_Enable(_SPI);
    LL_SPI_StartMasterTransfer(_SPI);
#endif

    while (size--) {
#if defined(SPI_SR_TXP)
      while (!LL_SPI_IsActiveFlag_TXP(_SPI));
#else
      while (!LL_SPI_IsActiveFlag_TXE(_SPI));
#endif
      LL_SPI_TransmitData8(_SPI, tx_buf ? *tx_buf++ : 0XFF);

#if defined(SPI_SR_RXP)
      while (!LL_SPI_IsActiveFlag_RXP(_SPI));
#else
      while (!LL_SPI_IsActiveFlag_RXNE(_SPI));
#endif
      if (rx_buffer) {
        *rx_buffer++ = LL_SPI_ReceiveData8(_SPI);
      } else {
        LL_SPI_ReceiveData8(_SPI);
      }
      if ((SPI_TRANSFER_TIMEOUT != HAL_MAX_DELAY) &&
          (HAL_GetTick() - tickstart >= SPI_TRANSFER_TIMEOUT)) {
        ret = SPI_TIMEOUT;
        break;
      }
    }

#if defined(SPI_IFCR_EOTC)
    // Add a delay before disabling SPI otherwise last-bit/last-clock may be truncated
    // See https://github.com/stm32duino/Arduino_Core_STM32/issues/1294
    // Computed delay is half SPI clock
    delayMicroseconds(obj->disable_delay);

    /* Close transfer */
    /* Clear flags */
    LL_SPI_ClearFlag_EOT(_SPI);
    LL_SPI_ClearFlag_TXTF(_SPI);
    /* Disable SPI peripheral */
    LL_SPI_Disable(_SPI);
#else
    /* Wait for end of transfer */
    while (LL_SPI_IsActiveFlag_BSY(_SPI));
#endif
  }
  return ret;
}

/******************************************************/
/* Low-level serial interface function implementation */
/******************************************************/

void inv_io_hal_board_init(void)
{
    // Initialize LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void inv_io_hal_configure_spi_speed(uint8_t spi_freq_mhz)
{
    spi_freq_hz_ = spi_freq_mhz * 1000000;
}

int inv_io_hal_init(struct inv_icm426xx_serif* serif)
{
    // Initialize CS pin and save PinName for fast access
    pinMode(BoardConfig::imu.spi.cs_pin, OUTPUT);
    digitalWrite(BoardConfig::imu.spi.cs_pin, HIGH);
    cs_pin_ = digitalPinToPinName(BoardConfig::imu.spi.cs_pin);

    // Set DWT ticks per uSec
    usTicks_ = SystemCoreClock / 1'000'000;

    // Save SPI instance handle and init SPI peripheral
    spi_hdl_ = spi_bus.getHandle();
    spi_bus.begin(SPISettings(spi_freq_hz_, MSBFIRST, SPI_MODE0));

    return 0;
}

int inv_io_hal_configure(struct inv_icm426xx_serif *serif)
{
    switch (serif->serif_type) {
    default:
        return -1;
    }
}

int inv_io_hal_read_reg(struct inv_icm426xx_serif *serif, uint8_t reg, uint8_t *rbuffer, uint32_t rlen)
{
    return inv_spi_bus_read_registers(reg, (uint8_t)rlen, rbuffer);
}

int inv_io_hal_write_reg(struct inv_icm426xx_serif * serif, uint8_t reg, const uint8_t* wbuffer, uint32_t wlen)
{
    int rc;

    for (uint32_t i = 0; i < wlen; i++) {
        rc = inv_spi_bus_write_register(reg + i, &wbuffer[i]);
        if (rc) {
            return rc;
        }
    }
    return 0;
}

void inv_spi_chip_select_setup_delay(void)
{
    // CS->CLK delay, MPU6000 - 8ns
    // CS->CLK delay, ICM42688P - 39ns
    DelayNs(39);
}

void inv_spi_chip_select_hold_time(void)
{
    // CLK->CS delay, MPU6000 - 500ns
    // CS->CLK delay, ICM42688P - 18ns
    DelayNs(18);
}

void inv_spi_bus_select_device(void)
{
    digitalWriteFast(cs_pin_, LOW);
    inv_spi_chip_select_setup_delay();
}

void inv_spi_bus_deselect_device(void)
{
    inv_spi_chip_select_hold_time();
    digitalWriteFast(cs_pin_, HIGH);
}

uint8_t inv_spi_transfer_byte(uint8_t txByte)
{
    uint8_t value = 0xFF;
    // spi_bus.transfer(&txByte, &value, 1);
    TransferLL(&txByte, &value, 1);
    return value;
}

int inv_spi_bus_read_registers(uint8_t addr, uint8_t count, uint8_t* data)
{
    inv_spi_bus_select_device();

    inv_spi_transfer_byte((addr | 0x80));
    for (uint8_t i = 0; i < count; i++) {
        // spi_bus.transfer(NULL, &data[i], 1);
        TransferLL(NULL, &data[i], 1);
    }

    inv_spi_bus_deselect_device();

    return 0;
}

int inv_spi_bus_write_register(uint8_t reg, const uint8_t* data)
{
    inv_spi_bus_select_device();

    inv_spi_transfer_byte(reg);
    inv_spi_transfer_byte(*data);

    inv_spi_bus_deselect_device();

    return 0;
}

#ifdef __cplusplus
}
#endif