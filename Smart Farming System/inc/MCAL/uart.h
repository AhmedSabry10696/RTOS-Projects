/*
 * UART.h
 *
 * Created: 2/19/2017 8:08:52 PM
 *  Author: Ahmed Sabry
 */  

#ifndef UART_H_
#define UART_H_

#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"

void UART_init(void);

void UART_sendByte(const uint8 data);

uint8 UART_recieveByte(void);

ERROR_t UART_receiveByte_NonBlocking(uint8 * pData);

void UART_sendString(const char *Str);

void UART_receiveString(char *Str); /* Receive until '#' */

#endif /* UART_H_ */
