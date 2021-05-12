/**
 * @file micro_config.h
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief micro controller configuratins
 * @version 0.1
 * @date 2019-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef MICRO_CONFIG_H_
#define MICRO_CONFIG_H_


#ifndef F_CPU
    /* 8 MHz Clock frequency */
    #define F_CPU 8000000UL
#endif  

#include <avr/io.h>
#include <util/delay.h>

#endif /* MICRO_CONFIG_H_ */
