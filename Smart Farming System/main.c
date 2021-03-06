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


#include "app.h"

int main(void)
{
	/* os init */
	System_Init();

	/* OS Object Creation */
	egControl = xEventGroupCreate();
	egDisplay = xEventGroupCreate();
	bsCheck = xSemaphoreCreateBinary();

	/* tasks creation with different priorities */
	xTaskCreate(T_Display, 	 NULL, 200, NULL, 2, NULL);
	xTaskCreate(T_Sensing, 	 NULL, 100,  NULL, 3, NULL);
	xTaskCreate(T_Terminal,  NULL, 150, NULL, 4, NULL);
	xTaskCreate(T_SysCheck,  NULL, 100,  NULL, 5, NULL);
	xTaskCreate(T_Control,	 NULL, 150, NULL, 6, NULL);

	/* start scheduling */
	vTaskStartScheduler();
}


/**
 * @brief Control heater, cooler and water pump
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
			/* update heater state */
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
			/* update cooler state */ 
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
			/* update water pump state */
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
	}
}

/**
 * @brief check current readings form the sensor with threshold values */
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

			xEventGroupSetBits(egControl, E_COOLER | E_HEATER);
			vTaskDelay(10);
			xEventGroupClearBits(egControl, E_COOLER | E_HEATER);

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
		}

	}
}

/**
 * @brief take input from user
 * 
 * @param pvParam 
 */
void T_Terminal(void* pvParam)
{
	uint8 data;
	uint8 strTTemp[4];
	uint8 strTHumi[4];
	
	/* clear two arrays */
	memset(strTTemp, 0, 3);
	memset(strTHumi, 0, 3);

	/* to differentiate what i am receiving */
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
						/* the data is 'C' configuration */
						if('C' == data)	
						{
							SFS.SystemState = ConfigState;
							/* clearing index to start saving from zero in next config */
							i = 0; 	
							/* clear temporary data for next config */
							memset(strTHumi, 0, 3);  
							/* config screen */
							xEventGroupSetBits(egDisplay, E_ConfigScreen); 
						}
					}
					
					else if (ConfigState == SFS.SystemState)
					{
						if('C' == data)	/* the data is 'C' cancell */
						{
							SFS.SystemState = MainState;
							/* Display main */
							xEventGroupSetBits(egDisplay, E_MainScreen); 
							/* clear temporary data for next config */
							memset(strTTemp, 0, 3);  
						}

						else if(data >= '0' && data <= '9')	/* the data is digit */
						{
							/* receive till the max 3 digits */
							if(i<3)	
							{
								strTTemp[i] = data;
								i++;
							}
						}
						
						/* the data is 'O' */
						else if( 'O' == data)	
						{
							/* Do not update the global struct if no data exist */
							if( 0 == atoi(strTTemp) )
							{
								/* clear temporary data */
								memset(strTTemp, 0, 3); 
							}
							
							else
							{
								/* update global threshold*/
								SFS.SensorThreshold.TempT = atoi(strTTemp);
								/* clear temporary data */
								memset(strTTemp, 0, 3); 

								/* give semaphore to system check */
								xSemaphoreGive(bsCheck);
								xEventGroupSetBits(egDisplay, E_TTUpdated);
							}

							i = 0;
							/* Go to Humidity receiving state */
							ReceivingState = HumiReceiving; 	
							/* in both situation, move the cursor to humidity*/
							xEventGroupSetBits(egDisplay, E_Next); 
						}
						
						/* the data is 'N' */
						else if( 'N' == data)	
						{
							/* to start from zero in Humidity receiving */
							i = 0; 
							/* clear temporary data */
							memset(strTTemp, 0, 3);  
							/* Go to Humidity receiving state */
							ReceivingState = HumiReceiving; 	
							/* tell the display to move the cursor */
							xEventGroupSetBits(egDisplay, E_Next); 
						}
						
					} /* end IF ConfigState */
						
				} /* end if Data exist in uart */
			}break;
			
			case HumiReceiving:
			{
				/* if new data exist on terminal */
				if(E_OK == UART_receiveByte_NonBlocking(&data)) 
				{
					/* the data is 'C' cancell */
					if('C' == data)	
					{
						ReceivingState = TempReceiving;
						/* clearing index to start saving from zero in next config */
						i = 0; 	
						/* clear temporary data for next config */
						memset(strTHumi, 0, 3);  
						SFS.SystemState = MainState;
						/* Display main */
						xEventGroupSetBits(egDisplay, E_MainScreen); 
					}
					
					/* the data is digit */
					else if( '9' >= data && '0' <= data)	
					{
						/* receive till max 3 digits */
						if(3 > i)	
						{
							strTHumi[i] = data;
							i++;
						}
									
					}
					
					/* the data is 'O' */
					else if('O' == data)	
					{
						/* if there is no data received in humi */
						if( 0 == atoi(strTHumi) )  
						{
							/* clear temporary data */
							memset(strTTemp, 0, 3); 
						}
						else
						{
							/* update global threshold*/
							SFS.SensorThreshold.HumiT = atoi(strTHumi);	
							/* clear temporary data */
							memset(strTTemp, 0, 3);  
							/* give semaphore to system check */
							xSemaphoreGive(bsCheck);
							xEventGroupSetBits(egDisplay, E_HTUpdated);
							vTaskDelay(500);
						}

						/* next state */
						ReceivingState = TempReceiving; 
						SFS.SystemState = MainState;

						/* in both cases go to main screen */
						xEventGroupSetBits(egDisplay, E_MainScreen); 

					}
					
					/* the data is 'N' */
					else if('N' == data)	
					{
						/* set receiving state in next time to temp */
						ReceivingState = TempReceiving; 	
						SFS.SystemState = MainState;

						xEventGroupSetBits(egDisplay, E_MainScreen);
						/* clear temporary data */
						memset(strTTemp, 0, 3); 
					}
				} /* end of if data exist in uart */
			}break;

			default:
				break;
		}	/* end of switch case */
	vTaskDelay(50);
	}
}

/**
 * @brief reading sensors data task
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
				/* give semaphore to system check */
				xSemaphoreGive(bsCheck);
				xEventGroupSetBits(egDisplay,E_TUpdated);
			}
		}
		if(E_OK == Humi_u16_Read(&humiValue))
		{
			if ( SFS.SensorData.HumiData!= humiValue )
			{
				SFS.SensorData.HumiData = humiValue;
				/* give semaphore to system check */
				xSemaphoreGive(bsCheck);
				xEventGroupSetBits(egDisplay,E_HUpdated);
			}
		}
		vTaskDelay(500);
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
					LCD_goToRowColumn(3,0);
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
				}
								
		}
	}
}

/**
 * @brief system initialization */
 * 
 */
void System_Init(void)
{
	/* lcd init */
	LCD_init();

	/* ADC init */
	ADC_init();

	/* MOTORS  directions */
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

