/**
 * @file sensors.c
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief temperature and humidity sensors dirvers
 * @version 0.1
 * @date 2021-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "sensors.h"
#include "adc.h"

ERROR_t TEMP_u16_Read(uint16 * pTemp)
{
	uint16 adc_read;

	adc_read = ADC_readChannel(TEMP_SENSOR_CH);

	/* convert adc value to temperature */
	*pTemp = (uint16)(adc_read * 48.80) / 100;

	return E_OK;
}

ERROR_t Humi_u16_Read(uint16 * pHumi)
{
	uint16 adc_read;

	/* convert adc value to humidity value */
	adc_read = ADC_readChannel(HUMI_SENSOR_CH);
	
	*pHumi = (uint16)(adc_read * 48.80) / 100;

	return E_OK;
}
