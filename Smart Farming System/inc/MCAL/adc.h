/**
 * @file adc.h
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief ADC header file
 * @version 0.1
 * @date 2019-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ADC_H_
#define ADC_H_

#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"

/*
 * Description :
 * Function responsible for initialize the ADC driver.
 */
void ADC_init(void);

/*
 * Description :
 * Function responsible for read analog data from a certain ADC channel
 * and convert it to digital using the ADC driver.
 */
uint16 ADC_readChannel(uint8 channel_num);

#endif /* ADC_H_ */
