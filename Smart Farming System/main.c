/**
 * @file main.c
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief Smart Farming Project based on FREE RTOS and ATMEGA 32
 * @version 0.1
 * @date 2021-04-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

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

/* MOTORs*/
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
typedef enum  {MainState, ConfigState} SystemState_t ;


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


int main(void)
{
	/* os init */
	System_Init();

	/* OS Object Creation */
	egControl = xEventGroupCreate();
	egDisplay = xEventGroupCreate();
	bsCheck = xSemaphoreCreateBinary();

	/* tasks creation with different periorities */
	xTaskCreate(T_Display, 	 NULL, 200, NULL, 2, NULL);
	xTaskCreate(T_Sensing, 	 NULL, 100,  NULL, 3, NULL);
	xTaskCreate(T_Terminal,  NULL, 150, NULL, 4, NULL);
	xTaskCreate(T_SysCheck,  NULL, 100,  NULL, 5, NULL);
	xTaskCreate(T_Control,	 NULL, 150, NULL, 6, NULL);

	/* start scheduling */
	vTaskStartScheduler();
}


/**
 * @brief Control task
 * 
 * @param pvParam 
 */
void T_Control(void* pvParam)
{
		
	while(1)
	{

		ebControlBits = xEventGroupWaitBits(egControl, E_CONTROLMASK , 0, 0, portMAX_DELAY);

		if ( (ebControlBits & E_HEATER) == E_HEATER )
		{
			if(Motors_State.Heater == ON)
			{
				SET_BIT(PORTD,HEATER);
			}
			else
			{
				CLEAR_BIT(PORTD,HEATER);
			}
		}

		if( (ebControlBits & E_COOLER) == E_COOLER )
		{
			if(Motors_State.Cooler == ON)
			{
				SET_BIT(PORTD,COOLER);
			}
			else
			{
				CLEAR_BIT(PORTD,COOLER);
			}
		}


		if( (ebControlBits & E_PUMP) == E_PUMP )
		{
			if(Motors_State.Water_Pump == ON)
			{
				SET_BIT(PORTD,WATER_PUMP);
			}
			else
			{
				CLEAR_BIT(PORTD,WATER_PUMP);
			}
		}
		vTaskDelay(5);
	} /* end while (1) */
}

/**
 * @brief System check task
 * 
 * @param pvParam 
 */
void T_SysCheck(void* pvParam)
{
	/* initial defaults */
	xEventGroupSetBits(egDisplay, E_MainScreen); 
	xEventGroupClearBits(egControl, E_CONTROLMASK);

	while(1)
	{
		if(xSemaphoreTake(bsCheck, portMAX_DELAY))
		{
			if(SFS.SensorData.TempData > SFS.SensorThreshold.TempT)
			{
				Motors_State.Cooler = ON;
				Motors_State.Heater = OFF;
			}
			else if(SFS.SensorData.TempData < SFS.SensorThreshold.TempT)
			{
				Motors_State.Cooler = OFF;
				Motors_State.Heater = ON;
			}
			else  /* when equal */
			{
				Motors_State.Cooler = OFF;
				Motors_State.Heater = OFF;
			}

			xEventGroupSetBits(egControl, E_COOLER | E_HEATER);		/* TODO */
			vTaskDelay(10);
			xEventGroupClearBits(egControl, E_COOLER | E_HEATER);	/* TODO */

			if(SFS.SensorData.HumiData >= SFS.SensorThreshold.HumiT)
			{
				Motors_State.Water_Pump = OFF;
			}
			else
			{
				Motors_State.Water_Pump = ON;
			}

			xEventGroupSetBits(egControl, E_PUMP);
			vTaskDelay(10);
			xEventGroupClearBits(egControl, E_PUMP);

			xEventGroupSetBits(egDisplay, E_MotorState); 
		} /* end of if semaphore */

	} /* end of while 1 */
}

/**
 * @brief terminal task
 * 
 * @param pvParam 
 */
void T_Terminal(void* pvParam)
{
	uint8 data;
	uint8 strTTemp[4];
	uint8 strTHumi[4];
	
	memset(strTTemp, 0, 3);
	memset(strTHumi, 0, 3);

	static enum {TempReceiving = 13, HumiReceiving} ReceivingState;
	
	/* Default entry point */
	ReceivingState = TempReceiving; 
	uint8 i = 0;

	while(1)
	{
		switch (ReceivingState)
		{
			case TempReceiving:
			{
				if(E_OK == UART_receiveByte_NonBlocking(&data))
				{
					if(MainState == SFS.SystemState )
					{
						/* the data is 'C' */
						if('C' == data)	
						{
							SFS.SystemState = ConfigState;
							i = 0; 	/* clearing index to start saving from zero in next config */
							memset(strTHumi, 0, 3);  /* clear temporary data for next config */
							xEventGroupSetBits(egDisplay, E_ConfigScreen); /* main screen */
						}
						else
						{
							
						}
					}
					
					else if (ConfigState == SFS.SystemState)
					{
						if('C' == data)	/* the data is 'C' */
						{
							SFS.SystemState = MainState;
							xEventGroupSetBits(egDisplay, E_MainScreen); /* Display main */
							memset(strTTemp, 0, 3);  /* clear temporary data for next config */

						}

						else if(data >= '0' && data <= '9')	/* the data is digit */
						{
							if(i<3)	/* if we still did`t receive the third digit */
							{
								strTTemp[i] = data;
								i++;
							}
						}
						
						else if( 'O' == data)	/* the data is 'O' */
						{
							if( 0 == atoi(strTTemp) )	/* FIXME *//* if u forget to clear the buffer in some conditions, an error may appear here */
							{
								/* Do not update the global struct if no data exist */
								memset(strTTemp, 0, 3);  /* clear temporary data */
							}
							
							else
							{
								SFS.SensorThreshold.TempT = atoi(strTTemp);	/* update global threshold*/
								memset(strTTemp, 0, 3);  /* clear temporary data */
								xSemaphoreGive(bsCheck);
								xEventGroupSetBits(egDisplay, E_TTUpdated);
							}

							i = 0;
							ReceivingState = HumiReceiving; 	/* Go to Humidity receiving state */
							xEventGroupSetBits(egDisplay, E_Next); /* in both situation, move the cursor to humidity*/

						}
						
						else if( 'N' == data)	/* the data is 'N' */
						{
							i = 0; /* to start from zero in Humidity receiving */
							memset(strTTemp, 0, 3);  /* clear temporary data */
							ReceivingState = HumiReceiving; 	/* Go to Humidity receiving state */
							xEventGroupSetBits(egDisplay, E_Next); /* tell the display to move the cursor */
						}
						
						else
						{
							/* Ignore, Unpredicted data entered*/
						}
						
					} /* end IF ConfigState */
						
				} /* end if Data exist in uart */
			}break;
			
			case HumiReceiving:
			{
				if(E_OK == UART_receiveByte_NonBlocking(&data)) /* if new data exist on terminal */
				{
					if('C' == data)	/* the data is 'C' */
					{
						ReceivingState = TempReceiving;
						i = 0; 	/* clearing index to start saving from zero in next config */
						memset(strTHumi, 0, 3);  /* clear temporary data for next config */
						SFS.SystemState = MainState;
						xEventGroupSetBits(egDisplay, E_MainScreen); /* Display main */
					}
					
					else if( '9' >= data && '0' <= data)	/* the data is digit */
					{
						if(3 > i)	/* if we still did`t receive the third digit */
						{
							strTHumi[i] = data;
							i++;
						}
									
					}
					
					else if('O' == data)	/* the data is 'O' */
					{
						if( 0 == atoi(strTHumi) )  /* if u forget to clear the buffer in some conditions, an error may appear here */
						{
							memset(strTTemp, 0, 3);  /* clear temporary data */
						}
						
						else
						{
							SFS.SensorThreshold.HumiT = atoi(strTHumi);	/* update global threshold*/
							memset(strTTemp, 0, 3);  /* clear temporary data */
							xSemaphoreGive(bsCheck);
							xEventGroupSetBits(egDisplay, E_HTUpdated);
							vTaskDelay(500);
						}

						ReceivingState = TempReceiving; /* next state */
						SFS.SystemState = MainState;
						xEventGroupSetBits(egDisplay, E_MainScreen); /* in both cases go to main screen */

					}
					
					else if('N' == data)	/* the data is 'N' */
					{
						ReceivingState = TempReceiving; 	/* Go to temp receiving state */
						SFS.SystemState = MainState;
						xEventGroupSetBits(egDisplay, E_MainScreen);
						memset(strTTemp, 0, 3);  /* clear temporary data */
					}
					
					else
					{
						/* Ignore, Unpredicted data entered*/
					}
				} /* end of if data exist in uart */
				
				else
				{
					/* if no data exist */
				}
			}break;

			default:
				break;
		}	/* end of switch case */
	vTaskDelay(50);  /* Task periodicity is 50ms */
	} /* end of while 1 */
}

/**
 * @brief Sensing task
 * 
 * @param pvParam 
 */
void T_Sensing(void* pvParam)
{
	uint8 tempValue = 0;
	uint8 humiValue = 0;

	while(1)
	{
		if(E_OK == TEMP_u16_Read(&tempValue))
		{
			if(  SFS.SensorData.TempData != tempValue )
			{
				SFS.SensorData.TempData= tempValue;
				xSemaphoreGive(bsCheck); /* sys check */
				xEventGroupSetBits(egDisplay,E_TUpdated);
			}

		}
		else
		{
						
		}

		if(E_OK == Humit_u16_Read(&humiValue))
		{
			if ( SFS.SensorData.HumiData!= humiValue )
			{
				SFS.SensorData.HumiData = humiValue;
				xSemaphoreGive(bsCheck);
				xEventGroupSetBits(egDisplay,E_HUpdated);
			}

		}
		
		else
		{
						
		}
		vTaskDelay(500);  	/* Task periodicity is 1 sec */
	}
}

/**
 * @brief Display task
 * 
 * @param pvParam 
 */
void T_Display(void* pvParam)
{
	while(1)
	{
		ebDisplayBits = xEventGroupWaitBits(egDisplay,E_DISPLAYMASK,1,0,portMAX_DELAY);

		if( (ebDisplayBits & E_MainScreen) == E_MainScreen)
		{					
			if( MainState == SFS.SystemState )
			{
				LCD_clearScreen();
				LCD_displayString(LCD_MAIN_SCREEN_L1);

				LCD_goToRowColumn(0,LCD_TEMP_COL);
				LCD_displayString("   ");
				LCD_goToRowColumn(0,LCD_TEMP_COL);
				LCD_intgerToString(SFS.SensorData.TempData);

				LCD_goToRowColumn(0,LCD_HUMI_COL);
				LCD_displayString("   ");
				LCD_goToRowColumn(0,LCD_HUMI_COL);
				LCD_intgerToString(SFS.SensorData.HumiData);

				LCD_goToRowColumn(1,0);
				LCD_displayString(LCD_MAIN_SCREEN_L2);

				LCD_goToRowColumn(1,LCD_TEMP_COL);
				LCD_displayString("   ");
				LCD_goToRowColumn(1,LCD_TEMP_COL);
				LCD_intgerToString(SFS.SensorThreshold.TempT);

				LCD_goToRowColumn(1,LCD_HUMI_COL);
				LCD_displayString("   ");
				LCD_goToRowColumn(1,LCD_HUMI_COL);
				LCD_intgerToString(SFS.SensorThreshold.HumiT);

				LCD_goToRowColumn(2,0);
				LCD_displayString(LCD_MAIN_SCREEN_L3);
							
				if(Motors_State.Water_Pump == ON)
				{
					LCD_goToRowColumn(2,LCD_PUMP_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(2,LCD_PUMP_COL);
					LCD_displayString("ON");
				}
				else
				{
					LCD_goToRowColumn(2,LCD_PUMP_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(2,LCD_PUMP_COL);
					LCD_displayString("OFF");
				}

				if(Motors_State.Heater == ON)
				{
					LCD_goToRowColumn(2,LCD_HEATER_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(2,LCD_HEATER_COL);
					LCD_displayString("ON");
				}
				else
				{
					LCD_goToRowColumn(2,LCD_HEATER_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(2,LCD_HEATER_COL);
					LCD_displayString("OFF");
				}

				if(Motors_State.Cooler == ON)
				{
					LCD_goToRowColumn(2,LCD_COOLER_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(2,LCD_COOLER_COL);
					LCD_displayString("ON");
				}
				else
				{
					LCD_goToRowColumn(2,LCD_COOLER_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(2,LCD_COOLER_COL);
					LCD_displayString("OFF");
				}
										
				LCD_goToRowColumn(3,0);
				LCD_displayString(LCD_MAIN_SCREEN_L4);
							
				} /*end of if MainState*/
						
			}

			if( (ebDisplayBits & E_ConfigScreen) == E_ConfigScreen)
			{
				if(ConfigState == SFS.SystemState)
				{
					LCD_clearScreen();
					LCD_displayString(LCD_CONFIG_SCREEN_L1);
					LCD_displayString(LCD_CONFIG_SCREEN_L2);
					LCD_displayString(LCD_CONFIG_SCREEN_L4);

					LCD_goToRowColumn(0,LCD_CONFIG_COL);
					LCD_sendCommand(CURSOR_BLINK);
				}

			}
						
			if( (ebDisplayBits & E_TTUpdated) == E_TTUpdated)
			{
				if(ConfigState == SFS.SystemState)
				{
					LCD_goToRowColumn(0,LCD_CONFIG_COL);
					LCD_sendCommand(CURSOR_OFF);
					LCD_intgerToString(SFS.SensorThreshold.TempT);
				}
			}

			if( (ebDisplayBits & E_HTUpdated) == E_HTUpdated)
			{
				if(ConfigState == SFS.SystemState)
				{
					LCD_goToRowColumn(2,LCD_CONFIG_COL);
					LCD_sendCommand(CURSOR_OFF);
					LCD_intgerToString(SFS.SensorThreshold.HumiT);
				}
			}

			if( (ebDisplayBits & E_Next) == E_Next)
			{
				if(ConfigState == SFS.SystemState)
				{
					LCD_goToRowColumn(2,LCD_CONFIG_COL);
					LCD_sendCommand(CURSOR_BLINK);
				}
			}

			if( (ebDisplayBits & E_TUpdated) == E_TUpdated)
			{
				if(MainState == SFS.SystemState)
				{
					LCD_goToRowColumn(0,LCD_TEMP_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(0,LCD_TEMP_COL);
					LCD_intgerToString(SFS.SensorData.TempData);
				}
			}

			if( (ebDisplayBits & E_HUpdated) == E_HUpdated)
			{
				if(MainState == SFS.SystemState)
				{
					LCD_goToRowColumn(0,LCD_HUMI_COL);
					LCD_displayString("   ");
					LCD_goToRowColumn(0,LCD_HUMI_COL);
					LCD_intgerToString(SFS.SensorData.HumiData);
				}
			}

			if(MainState == SFS.SystemState)
			{
				if( (ebDisplayBits & E_MotorState) == E_MotorState)
				{
					if(Motors_State.Water_Pump == ON)
					{
						LCD_goToRowColumn(2,LCD_PUMP_COL);
						LCD_displayString("   ");
						LCD_goToRowColumn(2,LCD_PUMP_COL);
						LCD_displayString("ON");
					}
					else
					{
						LCD_goToRowColumn(2,LCD_PUMP_COL);
						LCD_displayString("   ");
						LCD_goToRowColumn(2,LCD_PUMP_COL);
						LCD_displayString("OFF");
					}

					if(Motors_State.Heater == ON)
					{
						LCD_goToRowColumn(2,LCD_HEATER_COL);
						LCD_displayString("   ");
						LCD_goToRowColumn(2,LCD_HEATER_COL);
						LCD_displayString("ON");
					}
					else
					{
						LCD_goToRowColumn(2,LCD_HEATER_COL);
						LCD_displayString("   ");
						LCD_goToRowColumn(2,LCD_HEATER_COL);
						LCD_displayString("OFF");
					}

					if(Motors_State.Cooler == ON)
					{
						LCD_goToRowColumn(2,LCD_COOLER_COL);
						LCD_displayString("   ");
						LCD_goToRowColumn(2,LCD_COOLER_COL);
						LCD_displayString("ON");
					}
					else
					{
						LCD_goToRowColumn(2,LCD_COOLER_COL);
						LCD_displayString("   ");
						LCD_goToRowColumn(2,LCD_COOLER_COL);
						LCD_displayString("OFF");
					}
				} /* if motorState*/
								
		}
	}
}


void System_Init(void)
{
	/* lcd init */
	LCD_init();

	/* ADC init */
	ADC_init();

	/* MOTORS */
	SET_BIT(DDRD,WATER_PUMP);
	SET_BIT(DDRD,HEATER);
	SET_BIT(DDRD,COOLER);

	/* initialize to defaults Zero*/
	CLEAR_BIT(PORTD,WATER_PUMP);
	CLEAR_BIT(PORTD,HEATER);
	CLEAR_BIT(PORTD,COOLER);

	/*Defaults initialize */
	SFS.SensorData.TempData = 20;
	SFS.SensorData.HumiData = 30;
	SFS.SensorThreshold.TempT = 20;
	SFS.SensorThreshold.HumiT = 30;

	/* uart init*/
	UART_init();
	UART_sendString("System started\r\n");
}

