#include "sensors.h"
#include "adc.h"

ERROR_t TEMP_u16_Read(uint16 * pTemp)
{
	uint16 adc_read;

	adc_read = ADC_readChannel(TEMP_SENSOR_CH);

	*pTemp = (uint16)(adc_read * 48.80) / 100;

	return E_OK;
}

ERROR_t Humit_u16_Read(uint16 * pHumi)
{
	uint16 adc_read;

	adc_read = ADC_readChannel(HUMI_SENSOR_CH);
	
	*pHumi = (uint16)(adc_read * 48.80) / 100;

	return E_OK;
}
