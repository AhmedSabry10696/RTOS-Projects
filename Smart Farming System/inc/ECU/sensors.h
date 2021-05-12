/**
 * @file sensors.h
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief temperature and humidity sensor header file
 * @version 0.1
 * @date 2021-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef HAL_SENSORS_SENSORS_H_
#define HAL_SENSORS_SENSORS_H_

//#include "ADC_interface.h"
#include "std_types.h"

/* define pins for temperature and humidity seneors */
#define TEMP_SENSOR_CH 0
#define HUMI_SENSOR_CH 1

/**
 * @brief read temperature value
 * 
 * @param pTemp store temp value in this pointer
 * @return ERROR_t result of reading operation E_OK, E_NOK, PENDING
 */
ERROR_t TEMP_u16_Read(uint16 * pTemp);

/**
 * @brief read humidity value
 * 
 * @param pHumi store humi value in this pointer
 * @return ERROR_t result of reading operation E_OK, E_NOK, PENDING
 */
ERROR_t Humi_u16_Read(uint16 * pHumi);

#endif /* HAL_SENSORS_SENSORS_H_ */
