#ifndef HAL_SENSORS_SENSORS_H_
#define HAL_SENSORS_SENSORS_H_

//#include "ADC_interface.h"
#include "std_types.h"

#define TEMP_SENSOR_CH 0
#define HUMI_SENSOR_CH 1

ERROR_t TEMP_u16_Read(uint16 * pTemp);
ERROR_t Humit_u16_Read(uint16 * pHumi);

#endif /* HAL_SENSORS_SENSORS_H_ */
