/**
 * @file lcd.c
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief lcd driver
 * @version 0.1
 * @date 2019-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "stdlib.h"
#include "lcd.h"

void LCD_init(void)
{
	/* Configure the control pins(E,RS,RW) as output pins */
	LCD_CTRL_PORT_DIR |= (1<<E) | (1<<RS) | (1<<RW); 
	
	#if (DATA_BITS_MODE == 4)
		#ifdef UPPER_PORT_PINS
			/* Configure the highest 4 bits of the data port as output pins */
			LCD_DATA_PORT_DIR |= 0xF0; 
		#else
			/* Configure the lowest 4 bits of the data port as output pins */
			LCD_DATA_PORT_DIR |= 0x0F; 
		#endif		 
		/* initialize LCD in 4-bit mode */
		LCD_sendCommand(FOUR_BITS_DATA_MODE); 
		/* use 2-line lcd + 4-bit Data Mode + 5*7 dot display Mode */
		LCD_sendCommand(TWO_LINE_LCD_Four_BIT_MODE); 
	#elif (DATA_BITS_MODE == 8)
		/* Configure the data port as output port */ 
		LCD_DATA_PORT_DIR = 0xFF;
		/* use 2-line lcd + 8-bit Data Mode + 5*7 dot display Mode */
		LCD_sendCommand(TWO_LINE_LCD_Eight_BIT_MODE);
	#endif
	
	/* cursor off */
	LCD_sendCommand(CURSOR_OFF); 

	/* clear LCD at the beginning */
	LCD_sendCommand(CLEAR_COMMAND); 
}

void LCD_sendCommand(uint8 command)
{
	/* Instruction Mode RS = 0 */
	CLEAR_BIT(LCD_CTRL_PORT,RS); 
	/* write data to LCD so RW = 0 */
	CLEAR_BIT(LCD_CTRL_PORT,RW);
	/* delay for processing Tas = 50 ns */
	_delay_ms(1); 
	/* Enable LCD E = 1 */
	SET_BIT(LCD_CTRL_PORT,E); 
	/* delay for processing Tpw - Tdws = 190 ns */
	_delay_ms(1); 
	#if (DATA_BITS_MODE == 4)
		/* out the highest 4 bits of the required command to the data bus D4 --> D7 */
		#ifdef UPPER_PORT_PINS
			LCD_DATA_PORT = (command & 0xF0); 
		#else 
			LCD_DATA_PORT = ((command >> 4) & 0x0F);
		#endif
			
		/* delay for processing Tdsw = 100ns */
		_delay_ms(1); 
		/* disable LCD E=0 */
		CLEAR_BIT(LCD_CTRL_PORT,E); 
		/* delay for processing Th = 13ns */
		_delay_ms(1); 
		/* Enable LCD E=1 */
		SET_BIT(LCD_CTRL_PORT,E); 
		/* delay for processing Tpw - Tdws = 190ns */
		_delay_ms(1); 
		
		/* out the lowest 4 bits of the required command to the data bus D4 --> D7 */
		#ifdef UPPER_PORT_PINS
			LCD_DATA_PORT = (command << 4) & 0xF0; 
		#else 
			LCD_DATA_PORT = (command & 0x0F);
		#endif
		
		/* delay for processing Tdsw = 100ns */
		_delay_ms(1); 
		/* disable LCD E=0 */
		CLEAR_BIT(LCD_CTRL_PORT,E); 
		/* delay for processing Th = 13ns */
		_delay_ms(1); 
	#elif (DATA_BITS_MODE == 8)
		/* out the required command to the data bus D0 --> D7 */ 
		LCD_DATA_PORT = command; 
		/* delay for processing Tdsw = 100 ns */
		_delay_ms(1); 
		/* disable LCD E = 0 */
		CLEAR_BIT(LCD_CTRL_PORT,E); 
		/* delay for processing Th = 13 ns */
		_delay_ms(1); 
	#endif	
}

void LCD_displayCharacter(uint8 data)
{
	/* Data Mode RS = 1 */
	SET_BIT(LCD_CTRL_PORT,RS); 
	/* write data to LCD so RW = 0 */
	CLEAR_BIT(LCD_CTRL_PORT,RW); 
	/* delay for processing Tas = 50 ns */
	_delay_ms(1); 
	/* Enable LCD E = 1 */
	SET_BIT(LCD_CTRL_PORT,E); 
	/* delay for processing Tpw - Tdws = 190 ns */
	_delay_ms(1);
	#if (DATA_BITS_MODE == 4)
		/* out the highest 4 bits of the required command to the data bus D4 --> D7 */
		#ifdef UPPER_PORT_PINS
			LCD_DATA_PORT = (data & 0xF0); 
		#else 
			LCD_DATA_PORT = ((data >> 4) & 0x0F);
		#endif
		
		/* delay for processing Tdsw = 100ns */
		_delay_ms(1); 
		/* disable LCD E=0 */
		CLEAR_BIT(LCD_CTRL_PORT,E);
		/* delay for processing Th = 13ns */
		_delay_ms(1); 
		/* Enable LCD E=1 */
		SET_BIT(LCD_CTRL_PORT,E); 
		/* delay for processing Tpw - Tdws = 190ns */
		_delay_ms(1); 
		
		/* out the lowest 4 bits of the required data to the data bus D4 --> D7 */
		#ifdef UPPER_PORT_PINS
			LCD_DATA_PORT = (data << 4) & 0xF0; 
		#else 
			LCD_DATA_PORT = (data & 0x0F);
		#endif
		
		/* delay for processing Tdsw = 100ns */
		_delay_ms(1); 
		/* disable LCD E=0 */
		CLEAR_BIT(LCD_CTRL_PORT,E); 
		/* delay for processing Th = 13ns */
		_delay_ms(1); 
	#elif (DATA_BITS_MODE == 8)
		/* out the required data to the data bus D0 --> D7 */
		LCD_DATA_PORT = data; 
		/* delay for processing Tdsw = 100 ns */
		_delay_ms(1); 
		/* disable LCD E = 0 */
		CLEAR_BIT(LCD_CTRL_PORT,E); 
		/* delay for processing Th = 13 ns */
		_delay_ms(1); 
	#endif	
}

void LCD_displayString(const char *Str)
{
	uint8 i = 0;
	while(Str[i] != '\0')
	{
		LCD_displayCharacter(Str[i]);
		i++;
	}
	/******* Another Method *************
	while((*Str) != '\0')
	{
		LCD_displayCharacter(*Str);
		Str++;
	}		
	***********************************/
}

void LCD_goToRowColumn(uint8 row,uint8 col)
{
	uint8 Address = 0;
	
	/* first of all calculate the required address */
	switch(row)
	{	
		/* row 0 */
		case 0: 
				Address=col;
				break;
		
		/* row 1 */
		case 1:
				Address=col+0x40;
				break;

		/* row 2 */
		case 2:
				Address=col+0x14;
				break;
		
		/* row 3 */
		case 3:
				Address=col+0x54;
				break;
	}					
	/* to write to a specific address in the LCD 
	 * we need to apply the corresponding command 0b10000000+Address */
	LCD_sendCommand(Address | SET_CURSOR_LOCATION); 
}

void LCD_displayStringRowColumn(uint8 row,uint8 col,const char *Str)
{
	/* go to to the required LCD position */
	LCD_goToRowColumn(row,col);
	/* display the string */
	LCD_displayString(Str); 
}

void LCD_intgerToString(int data)
{
	/* String to hold the asci result */
	char buff[6]; 
	/* 10 for decimal */
	itoa(data,buff,10); 
	LCD_displayString(buff);
}

void LCD_clearScreen(void)
{
	/* clear display */ 
	LCD_sendCommand(CLEAR_COMMAND); 
}	 
