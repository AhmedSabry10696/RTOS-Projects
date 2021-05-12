/**
 * @file lcd.h
 * @author Ahmed Sabry (ahmed.sabry10696@gmail.com)
 * @brief lcd driver header file
 * @version 0.1
 * @date 2019-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef LCD_H_
#define LCD_H_

#include "std_types.h"
#include "common_macros.h"
#include "micro_config.h"

/* LCD Data bits mode configuration */
#define DATA_BITS_MODE 8

/* Use higher 4 bits in the data port */
#if (DATA_BITS_MODE == 4)
#define UPPER_PORT_PINS
#endif

/* LCD HW Pins */
#define RS PD5
#define RW PD6
#define E  PD7
#define LCD_CTRL_PORT PORTD
#define LCD_CTRL_PORT_DIR DDRD
#define LCD_DATA_PORT PORTC
#define LCD_DATA_PORT_DIR DDRC

/* LCD Commands */
#define CLEAR_COMMAND 0x01
#define FOUR_BITS_DATA_MODE 0x02
#define TWO_LINE_LCD_Four_BIT_MODE 0x28
#define TWO_LINE_LCD_Eight_BIT_MODE 0x38
#define CURSOR_OFF 0x0C
#define CURSOR_ON 0x0E
#define CURSOR_BLINK 0x0F
#define SET_CURSOR_LOCATION 0x80 

/**
 * @brief send command to lcd
 * 
 * @param command command to send
 */
void LCD_sendCommand(uint8 command);

/**
 * @brief display char on lcd
 * 
 * @param data char to display
 */
void LCD_displayCharacter(uint8 data);

/**
 * @brief display string on lcd
 * 
 * @param Str string to display
 */
void LCD_displayString(const char *Str);

/**
 * @brief initialize lcd 
 * 
 */
void LCD_init(void);

/**
 * @brief clear lcd display
 * 
 */
void LCD_clearScreen(void);

/**
 * @brief display string at specific row and column
 * 
 * @param row row of beginning to display
 * @param col col of beginning to display 
 * @param Str string to display
 */
void LCD_displayStringRowColumn(uint8 row,uint8 col,const char *Str);

/**
 * @brief go to specific row and column in lcd display
 * 
 * @param row 
 * @param col 
 */
void LCD_goToRowColumn(uint8 row,uint8 col);

/**
 * @brief convert int to string and display it on lcd
 * 
 * @param data integer data to display on lcd
 */
void LCD_intgerToString(int data);

#endif /* LCD_H_ */
