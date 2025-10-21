/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2016-2016 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively “Software”) is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */
#ifndef _SYSTEM_INTERFACE_H_
#define _SYSTEM_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Invn/Drivers/Icm426xx/Icm426xxTransport.h"

#ifndef TO_MASK
#define TO_MASK(a) (1U << (unsigned)(a))
#endif

void inv_io_hal_board_init(void);
/* /!\ When used, the below function be should called before inv_io_hal_board_init */
void inv_io_hal_configure_spi_speed(uint8_t spi_freq_mhz);
int  inv_io_hal_init(struct inv_icm426xx_serif* serif);
int  inv_io_hal_configure(struct inv_icm426xx_serif* serif);
int  inv_io_hal_read_reg(struct inv_icm426xx_serif* serif, uint8_t reg, uint8_t* rbuffer, uint32_t rlen);
int  inv_io_hal_write_reg(struct inv_icm426xx_serif* serif, uint8_t reg, const uint8_t* wbuffer, uint32_t wlen);

#ifdef __cplusplus
}
#endif

#endif /* !_SYSTEM_INTERFACE_H_ */
