/** @file    micro_config.h
 ** @brief   micro controller configuration header file
 ** @author  Ahmed Sabry
 ** @date    22/1/2017
 ** @version 1.0
 */

#ifndef MICRO_CONFIG_H_
#define MICRO_CONFIG_H_


#ifndef F_CPU
#define F_CPU 8000000UL // 8 MHz Clock frequency
#endif  
#include <avr/io.h>
#include <util/delay.h>

#endif /* MICRO_CONFIG_H_ */
