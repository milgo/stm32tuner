#include "lcd_lib.h"

void LCD_SendNibble(uint8_t cmd)
{
	GPIO_ResetBits(GPIOA, LCD_D4 | LCD_D5 | LCD_D7);
	GPIO_ResetBits(GPIOB, LCD_D6);
	
	if((cmd & 0x1) == 0x1)
		GPIO_WriteBit(GPIOA, LCD_D4, (BitAction)1);
	if((cmd & 0x2) == 0x2)
		GPIO_WriteBit(GPIOA, LCD_D5, (BitAction)1);
	if((cmd & 0x4) == 0x4)
		GPIO_WriteBit (GPIOB, LCD_D6, (BitAction)1);
	if((cmd & 0x8) == 0x8)
		GPIO_WriteBit(GPIOA, LCD_D7, (BitAction)1);
}

void LCD_WriteCommon( uint8_t value ) 
{ 
	LCD_SendNibble(value);
	GPIO_SetBits(GPIOA, LCD_E);
	Delay(1); 
	GPIO_ResetBits(GPIOA, LCD_E);
	Delay(1);
} 

void LCD_WriteCtrl( uint8_t cmd ) 
{ 
	unsigned char high = cmd >> 4;
	unsigned char low = cmd & 0xF;
	
	GPIO_ResetBits(GPIOA, LCD_RS);
	LCD_WriteCommon(high); 
	Delay(10); 
	LCD_WriteCommon(low); 
	Delay(10); 
} 


void LCD_WriteData( uint8_t data ) 
{ 
	unsigned char high = data >> 4;
	unsigned char low = data & 0xF;
	
	GPIO_SetBits(GPIOA, LCD_RS);
	LCD_WriteCommon( high ); 
	LCD_WriteCommon( low );
}

void LCD_Init(void)
{
	/* Wait a bit after power-up */ 
	Delay(200); 

	GPIO_ResetBits(GPIOA, LCD_RW | LCD_E | LCD_D4 | LCD_D5 | LCD_D7);
	GPIO_ResetBits(GPIOB, LCD_D6);
	GPIO_ResetBits(GPIOB, LCD_RW);
	
	/* Initialize the LCD to 4-bit mode */
	LCD_WriteCtrl(3); 
	Delay(50); 
	LCD_WriteCtrl(3); 
	Delay(10); 
	LCD_WriteCtrl(3); 
	Delay(10); 
	LCD_WriteCtrl(2); 
	Delay(10); 
	
	/* Function Set */ 
	LCD_WriteCtrl(0x28);
	
	/* Display OFF */ 
	LCD_WriteCtrl(0x08);
	
	/* Display ON */ 
	LCD_WriteCtrl(0x0F);
	
	/* Entry mode */ 
	LCD_WriteCtrl(0x06);
	
	/* Clear Screen */ 
	LCD_WriteCtrl(0x01);
	Delay(100); 
	
	/* Cursor home */ 
	LCD_WriteCtrl(0x02);
	Delay(100); 
	
	LCD_WriteCtrl(0x0C);
}			

void LCD_WriteString(char* data, uint8_t nBytes)
{
	uint8_t i;

	// check to make sure we have a good pointer
	if (!data) return;
	
	// print data
	for(i=0; i<nBytes; i++)
	{
		if(data[i]==0)break;
		LCD_WriteData( data[i] );
	}
}

void LCD_WriteNumber(uint16_t n)
{
	LCD_WriteString(itoa(n), 3);
}

void LCD_WriteDecimal(float32_t f)
{
	int r, i = (int)(f * 10.0f);
	r = i % 10;
	i -= r;
	i /= 10;
	if(i < 10)LCD_WriteString(itoa(i), 1);
	else if(i < 100)LCD_WriteString(itoa(i), 2);
	else if(i < 1000)LCD_WriteString(itoa(i), 3);
	else if(i < 10000)LCD_WriteString(itoa(i), 4);
	else if(i < 100000)LCD_WriteString(itoa(i), 5);
	
	LCD_WriteData('.');
	LCD_WriteString(itoa(r), 1);
}

void LCD_GotoXY(uint8_t x, uint8_t y)
{
	uint8_t DDRAMAddr;

	switch(y)
	{
	case 0:  DDRAMAddr = LCD_LINE0_DDRAMADDR + x; break;
	case 1:  DDRAMAddr = LCD_LINE1_DDRAMADDR + x; break;
	case 2:  DDRAMAddr = LCD_LINE2_DDRAMADDR + x; break;
	case 3:  DDRAMAddr = LCD_LINE3_DDRAMADDR + x; break;
	default: DDRAMAddr = LCD_LINE0_DDRAMADDR + x; break;
	}

	LCD_WriteCtrl( (1<<LCD_DDRAM) | DDRAMAddr);
}

