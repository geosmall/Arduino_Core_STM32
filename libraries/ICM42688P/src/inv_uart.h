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

/** @defgroup Uart Uart
	@ingroup  Low_Level_Driver
	@{
*/
#ifndef __INV_UART_H__
#define __INV_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Available UART peripherals
 */
typedef enum uart_num {
	INV_UART_SENSOR_CTRL, /**< Just a fancier and hw independent name for UART0. It is used
	                                < to send command to/read data from sensors.
	                                < On samg55 it is mapped on CN6 USB port through FTDI.
	                                < UART0 has hw handshake capabilities. */
	INV_UART_LOG,         /**< Just a fancier and hw independent name for UART7. It is used
	                                < for SW traces.
	                                < On samg55 it is mapped through EDBG USB port.
	                                < */
}inv_uart_num_t;

/**
 * Possible states of the uart driver
 */
typedef enum {
	INV_UART_STATE_RESET = 0,
	INV_UART_STATE_IDLE,
	INV_UART_STATE_BUSY_TX,
	INV_UART_STATE_BUSY_RX
}inv_uart_state_t;

/**
 * Flow control
 */
typedef enum {
	INV_UART_FLOW_CONTROL_NONE    = 0,
	INV_UART_FLOW_CONTROL_RTS     = 1,
	INV_UART_FLOW_CONTROL_CTS     = 2,
	INV_UART_FLOW_CONTROL_RTS_CTS = 3
}inv_uart_flow_control_t;

/**
 * Uart driver error codes
 */
enum inv_uart_error{
	INV_UART_ERROR_SUCCES    = 0,    /** < No error */
	INV_UART_ERROR           = -1,   /** < Unspecified error */
	INV_UART_ERROR_BUSY      = -2,   /** < Uart driver busy */
	INV_UART_ERROR_MEMORY    = -3,   /** < Uart driver is out of memory */
	INV_UART_ERROR_BAD_ARG   = -4,   /** < Provided arguments are invalid */
};

/**
 * Uart transfer structure
 */
typedef struct {
	uint8_t * data;   /** < Pointer to the first data byte to be transmitted*/
	uint16_t  len;    /** < Number of bytes to be transmitted*/
}inv_uart_tx_transfer_t;

/**
 * Uart context receiver structure
 */
typedef struct {
	void * base;     /** < Pointer to uart manager structure */
	volatile uint8_t * buf;   /** < Pointer to the first data byte received */
	volatile uint16_t * idx;  /** < Pointer to the index of the first data byte to read */
} inv_uart_rx_context_t;

/** @brief UART initialization structure.
 *
 * Such a structure should be filled-in and passed to the uart_init function in order to 
 * initialize the uart peripheral. All fields need to be initialized in order to avoid 
 * unpredictable behavior.
 */
typedef struct inv_uart_init_struct {
	inv_uart_num_t              uart_num;                       /** < UART peripheral that we wish to initialize */
	uint8_t *                    tx_buffer;                      /** < Pointer to the start of the memory area allocated for the UART TX FIFO */
	uint8_t *                    rx_buffer;                      /** < Pointer to the start of the memory area allocated for the UART RX FIFO */
	uint16_t                     tx_size;                        /** < Buffer size for UART TX FIFO */
	uint16_t                     rx_size;                        /** < Buffer size for UART RX FIFO. 
	                                                                           Choose wisely this value, as overflow will lead to byte loss */
	int                          baudrate;                       /** < UART baudrate speed. For a list of supported baudrates refer to the device datasheet */
	inv_uart_flow_control_t     flow_ctrl;                      /** < Flow control as defined by uart_flow_control_t enum 
	                                                                      Note: UART2 and UART6 support only UART_FLOW_CONTROL_NONE */
	void                         (*tx_done_cb)(void * context);  /** < Callback executed when a DMA TX transaction is completed */
	void                         *tx_context;                    /** < Context passed to tx_done_cb */
}inv_uart_init_struct_t;

/** @brief      Initialize the UART peripheral according to 
 *              the parameters specified in the uart_init_struct
 *  @param[in]  uart Pointer to the uart_init_struct that contains
 *              the configuration information
 *  @return     0 on success
 *              INV_UART_ERROR_BUSY if a transaction is already ongoing
 *              INV_UART_ERROR if for an unknown reason the initialization failed
 *              INV_UART_ERROR_BAD_ARG if the uart_init_structure was incorrectly filled-in
 */
int inv_uart_init(inv_uart_init_struct_t * uart);

/** @brief      Transmits a character on the UART 
 *  @param[in]  uart UART peripheral
 *  @param[in]  ch Character to be printed
 *  @return     0 on success
 *              INV_UART_ERROR_MEMORY if the internal buffers are full
 *              INV_UART_ERROR_BUSY if another transaction is already ongoing
 *              INV_UART_ERROR if for an unknown reason the requested transaction did not start
 *              INV_UART_ERROR_BAD_ARG if one of the arguments is unsupported
 *  @note       The data that is to be transfered will be copied to the internal buffers.
 */
int inv_uart_putc(inv_uart_num_t uart, int ch);

/** @brief      Transmits an array of characters on the UART.
 *              As the length is sent as parameter, the array can contain zeros and 
 *              does no have to be NULL-terminated
 *  @param[in]  uart UART peripheral
 *  @param[in]  s Pointer to the array to be transmitted
 *  @param[in]  l Length of the array to be transmitted
 *  @return     0 on success
 *              INV_UART_ERROR_MEMORY if the internal buffers are full
 *              INV_UART_ERROR_BUSY if another transaction is already ongoing
 *              INV_UART_ERROR if for an unknown reason the requested transaction did not start
 *              INV_UART_ERROR_BAD_ARG if one of the arguments is unsupported
 *  @note       The data that is to be transfered will be copied to the internal buffers.
 */
int inv_uart_puts(inv_uart_num_t uart, const char * s, unsigned short l);

/** @brief      Transmits an array of characters on the UART.
 *              As the length is sent as parameter, the array can contain zeros and 
 *              does no have to be NULL-terminated.
 *  @param[in]  uart UART peripheral
 *  @param[in]  txfer Pointer to a inv_uart_tx_transfer_t structure that contains information
 *              on the transfer
 *  @return     0 on success
 *              INV_UART_ERROR_MEMORY if the internal buffers are full
 *              INV_UART_ERROR_BUSY if another transaction is already ongoing
 *              INV_UART_ERROR if for an unknown reason the requested transaction did not start
 *              INV_UART_ERROR_BAD_ARG if one of the arguments is unsupported
 *  @note       The data that is to be transfered will be copied to the internal buffers.
 */
int inv_uart_tx_txfer(inv_uart_num_t uart, inv_uart_tx_transfer_t * txfer);

/** @brief      Reads a character received by the UART peripheral
 *  @param[in]  uart     UART peripheral
 *  @return     First available character in the RX FIFO
 *              EOF if no character available in the RX FIFO
 */
int inv_uart_getc(inv_uart_num_t uart);

/** @brief      Returns the number of available bytes in the RX FIFO
 *  @param[in]  uart UART peripheral
 *  @return     Number of available bytes in the RX FIFO
 */
int inv_uart_available(inv_uart_num_t uart);

/** @brief      Returns the flow control configuration for the UART
 *  @param[in]  uart UART peripheral
 *  @return     Current flow control configuration as defined by the inv_uart_flow_control_t enum
 */
inv_uart_flow_control_t inv_uart_get_flow_control_configuration(inv_uart_num_t uart);

/** @brief      Returns the state of the TX module
 *  @param[in]  uart UART peripheral
 *  @return     The state of the TX module as defined by the inv_uart_state_t enum
 */
inv_uart_state_t inv_uart_tx_get_state(inv_uart_num_t uart);

/** @brief      Returns the state of the RX module
 *  @param[in]  uart UART peripheral
 *  @return     The state of the RX module as defined by the inv_uart_state_t enum
 */
inv_uart_state_t inv_uart_rx_get_state(inv_uart_num_t uart);

#ifdef __cplusplus
}
#endif

#endif // __INV_UART_H__

/** @} */
