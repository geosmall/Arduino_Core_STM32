#include "board.h"
#include "stm32_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void __libc_init_array(void);

WEAK void init(void)
{
  configIPClock();

  /* Initialize the HAL */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

#if defined(USBCON) && defined(USBD_USE_CDC)
  MX_USB_DEVICE_Init();
#endif
}

#ifdef __cplusplus
}
#endif
