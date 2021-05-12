/**
 * @file app.h
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief application header file
 * @version 0.1
 * @date 2021-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */


#ifndef APP_H_
#define APP_H_

/* include OS header file */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"
#include "stdlib.h"
#include "string.h"
#include "uart.h"
#include "lcd.h"
#include "adc.h"
#include "sensors.h"

/* Tasks /Functions Prototypes*/
void System_Init(void);
void T_Control(void* pvParam);
void T_SysCheck(void* pvParam);
void T_Terminal(void* pvParam);
void T_Sensing(void* pvParam);
void T_Display(void* pvParam);

/* used to trigger the T_Control task */
#define E_PUMP			(1<<0)		
#define E_HEATER		(1<<1)		
#define E_COOLER		(1<<2)		
#define E_CONTROLMASK 	(0b111)

/* used to trigger the T_Display task */
#define E_MainScreen	(1<<0)
#define E_ConfigScreen	(1<<1)
#define E_HUpdated		(1<<2)
#define E_TUpdated		(1<<3)
#define E_TTUpdated		(1<<4)	
#define E_HTUpdated		(1<<5)
#define E_MotorState	(1<<6) 	
#define E_Next			(1<<7)	
#define E_DISPLAYMASK 	(0b11111111)

/* Motores connections */
#define WATER_PUMP 		PIN2
#define HEATER	   		PIN3
#define COOLER	   		PIN4

/******************* Display ************************/
#define LCD_MAIN_SCREEN_L1		"T =    C    H =    %"
#define LCD_MAIN_SCREEN_L2		"TT=    C    TH=    %"
#define LCD_TEMP_COL			4
#define LCD_HUMI_COL			16

#define LCD_MAIN_SCREEN_L3		"H:      C:     P:   "
#define LCD_HEATER_COL			2
#define LCD_COOLER_COL			10
#define LCD_PUMP_COL			17

#define LCD_MAIN_SCREEN_L4		"  Configuration: C  "

#define LCD_CONFIG_SCREEN_L1	"TempThreshold:     C"
#define LCD_CONFIG_SCREEN_L2	"HumiThreshold:     %"
#define LCD_CONFIG_COL			14

#define LCD_CONFIG_SCREEN_L4	"OK:O Next:N Cancel:C"

EventGroupHandle_t egControl = NULL;
EventGroupHandle_t egDisplay = NULL;

EventBits_t ebControlBits;
EventBits_t ebDisplayBits;
SemaphoreHandle_t bsCheck;

/**
 * @brief Motor State enum
 * 
 */
typedef enum
{
	OFF,ON
}Motor;

/**
 * @brief System Motors
 * 
 */
struct
{
	Motor Water_Pump;
	Motor Heater;
	Motor Cooler;
}Motors_State;

/**
 * @brief System States enum
 * 
 */
typedef enum {MainState, ConfigState} SystemState_t ;


/**
 * @brief All system data
 * 
 */
struct
{
	/* system state */
	SystemState_t SystemState;

	/**
	 * @brief sensors data
	 * 
	 */
	struct
	{
		uint8 TempData;
		uint8 HumiData;
	} SensorData;

	/**
	 * @brief threshold values
	 * 
	 */
	struct
	{
		uint8 TempT;
		uint8 HumiT;
	} SensorThreshold;

} SFS;



#endif /* APP_H_ */