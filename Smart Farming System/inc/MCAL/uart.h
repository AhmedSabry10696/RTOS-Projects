/**
 * @file uart.h
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief UART header file
 * @version 0.1
 * @date 2021-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef UART_H_
#define UART_H_

#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"

/**
 * @brief initialize uart
 * 
 */
void UART_init(void);

/**
 * @brief send byte through uart 
 * 
 * @param data byte to send
 */
void UART_sendByte(const uint8 data);

/**
 * @brief receive byte through uart
 * 
 * @return uint8 received byte
 */
uint8 UART_receiveByte(void);

/**
 * @brief receive byte through uart without polling
 * 
 * @param pData received byte
 * @return ERROR_t result of receiving operation E_OK, E_NOK, PENDING
 */
ERROR_t UART_receiveByte_NonBlocking(uint8 * pData);

/**
 * @brief send string through UART
 * 
 * @param Str string to send
 */
void UART_sendString(const char *Str);


/**
 * @brief receive stirng through UART until '#' 
 * 
 * @param Str received string
 */
void UART_receiveString(char *Str);

#endif /* UART_H_ */
