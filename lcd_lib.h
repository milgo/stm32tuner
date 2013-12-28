#ifndef LCD_LIB
#define LCD_LIB

#include "arm_math.h"
#include "stm32f10x.h"
#include "main.h"
#include "itoa.h"

#define LCD_Clear() LCD_WriteCtrl(0x01)
#define LCD_Home() LCD_WriteCtrl(0x02)


#define LCD_RW 	GPIO_Pin_0
#define LCD_RS	GPIO_Pin_12
#define LCD_E		GPIO_Pin_11
#define LCD_D4	GPIO_Pin_9	
#define LCD_D5	GPIO_Pin_10
#define LCD_D6	GPIO_Pin_15	//PB!
#define LCD_D7	GPIO_Pin_8 

#define LCD_CLR             0	//DB0: clear display
#define LCD_HOME            1	//DB1: return to home position
#define LCD_ENTRY_MODE      2	//DB2: set entry mode
#define LCD_ENTRY_INC       1	//DB1: increment
#define LCD_ENTRY_SHIFT     0	//DB2: shift
#define LCD_ON_CTRL         3	//DB3: turn lcd/cursor on
#define LCD_ON_DISPLAY      2	//DB2: turn display on
#define LCD_ON_CURSOR       1	//DB1: turn cursor on
#define LCD_ON_BLINK        0	//DB0: blinking cursor
#define LCD_MOVE            4	//DB4: move cursor/display
#define LCD_MOVE_DISP       3	//DB3: move display (0-> move cursor)
#define LCD_MOVE_RIGHT      2	//DB2: move right (0-> left)
#define LCD_FUNCTION        5	//DB5: function set
#define LCD_FUNCTION_8BIT   4	//DB4: set 8BIT mode (0->4BIT mode)
#define LCD_FUNCTION_2LINES 3	//DB3: two lines (0->one line)
#define LCD_FUNCTION_10DOTS 2	//DB2: 5x10 font (0->5x7 font)
#define LCD_CGRAM           6	//DB6: set CG RAM address
#define LCD_DDRAM           7	//DB7: set DD RAM address

// cursor position to DDRAM mapping
#define LCD_LINE0_DDRAMADDR		0x00
#define LCD_LINE1_DDRAMADDR		0x40
#define LCD_LINE2_DDRAMADDR		0x14
#define LCD_LINE3_DDRAMADDR		0x54

void LCD_WriteData( uint8_t data );
void LCD_WriteCtrl( uint8_t cmd );
void LCD_WriteString( char* data, uint8_t nBytes );
void LCD_WriteNumber( uint16_t n );
void LCD_WriteDecimal(float32_t f);
void LCD_GotoXY( uint8_t x, uint8_t y );
void LCD_Init( void );

//void Delay(__IO uint32_t nTime);
#endif

