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

/** @defgroup GPIO GPIO
 *  @ingroup  Low_Level_Driver
 *  @{
 */
#ifndef _INV_GPIO_H_
#define _INV_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Enumeration of the GPIOs connected to the INV sensor:
       - INV_GPIO_REVG represents the GPIO between AP and ICM for rev G
       - INV_GPIO_REVI represents the GPIO between AP and ICM for rev I
       - INV_GPIO_ONBOARD_REVB represents the GPIO between AP and on-board ICM for rev B
       - INV_GPIO_BOARD_REVB_DB represents the GPIO between AP and DB ICM for rev B
       - INV_GPIO_BOARD_REVDPLUS represents the GPIO between AP and DB ICM for rev D+
*/
typedef enum gpio_board_type {
	INV_GPIO_BOARD_REVG = 0,
	INV_GPIO_BOARD_REVI = INV_GPIO_BOARD_REVG,
	INV_GPIO_BOARD_REVB = 1,
	INV_GPIO_BOARD_REVB_DB,
	INV_GPIO_BOARD_REVDPLUS,
	INV_GPIO_BOARD_MAX
} gpio_num_t;

enum gpio_inv_pin_num {
	INV_GPIO_INT1 = 0,         /* Connected to the INT1 pin of the Invensense chip. */
	INV_GPIO_INT2,             /* Connected to the INT2 pin of the Invensense chip. */
	INV_GPIO_FSYNC,            /* Connected to the FSYNC pin of the Invensense chip. */
	INV_GPIO_3RD_PARTY_INT1,   /* Connected to the interrupt pin of the 3rd party chip. */
	INV_GPIO_SW0_BUTTON,       /* Connected to the SW0 button. */
	INV_GPIO_CLKIN,            /* Output SLCK on PA17 pin. Set a hardware bridge to connect to CLKIN pin of the Invensense chip. */
	INV_GPIO_AD0,              /* Connected to the AD0 pin of the Invensense chip. */
	INV_GPIO_CSN,              /* Connected to the CSn pin of the Invensense chip. */
	INV_GPIO_MAX
};

/** @brief Force board revision
 *  @param[in] board_rev to define which revision, following gpio_num_t
 */
void inv_gpio_set_board_rev(gpio_num_t board_rev);

/** @brief Init the GPIO to output
 *  @param[in] pin_num to define which pin to configure
 */
void inv_gpio_init_pin_out(unsigned pin_num);

/** @brief Init the GPIO to input
 *  @param[in] pin_num to define which pin to configure
 */
void inv_gpio_init_pin_input(unsigned pin_num);

/** @brief Toggle GPIO state
 *  @param[in] pin_num which GPIO pin shall be toggled
 */
void inv_gpio_toggle_pin(unsigned pin_num);

/** @brief Get GPIO state
 *  @param[in]	pin_num which GPIO pin to read status
 *  @return		Pin status, -1 in case of pin number error
 */
int inv_gpio_get_status(unsigned pin_num);

/** @brief Set GPIO state high
*   @param[in] pin_num which GPIO pin shall be toggled
*/
void inv_gpio_set_pin_high(unsigned pin_num);

/** @brief Set GPIO state low
*   @param[in] pin_num which GPIO pin shall be toggled
*/
void inv_gpio_set_pin_low(unsigned pin_num);

/** @brief Output 32kHz clock to PA17
 *  @param[in]	pin_num which GPIO pin to output clock 
 *  @warning Pin must be PA17
 *  @warning This is not compatible with any call to gpio_sensor_irq_init(TO_MASK(GPIO_SENSOR_IRQ_D7))
 */
int inv_gpio_output_clk_on_pin(unsigned pin_num);

/** @brief Disable 32kHz output on PA17
 *  @param[in]	pin_num which GPIO pin to output clock
 *  @warning Pin must be PA17
 */
int inv_gpio_disable_clk_on_pin(unsigned pin_num);


/** @brief Init the sensor line interrupt to wake-up the MCU
 *  @param[in]	int_num         IRQ pin as defined by enum gpio_sensor_irq_num
 *  @param[in]	interrupt_cb    callback to call on interrupt
 *  @param[in]	context         context passed to callback
 */
void inv_gpio_sensor_irq_init(unsigned pin_num,
		void (*interrupt_cb)(void * context, unsigned int_num), void * context);

#ifdef __cplusplus
}
#endif

#endif /* _INV_GPIO_H_ */

/** @} */
