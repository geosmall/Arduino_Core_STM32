#include "stm32_def.h"
#include "board.h"
#include "clock.h"
#if defined(USBCON) && defined(USBD_USE_CDC)
  #include "usb_device.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// void __libc_init_array(void);

WEAK void board_init(void)
{
  configIPClock();

  /* Initialize the HAL */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

#if defined(USBCON) && defined(USBD_USE_CDC)
  USB_DEVICE_Init();
#endif
}

#ifdef __cplusplus
}
#endif
