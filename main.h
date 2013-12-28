#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "stm32f10x_it.h"
#include "stm32f10x_conf.h"
#include "arm_math.h"
#include "lcd_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define LENGTH_SAMPLES 			2048
#define FFT_SIZE						2048

/* TRIGGER VALUES */
#define TRIGGER_AT_DB				-3.0f
#define BATTERY_LOW_LEVEL		0.2f

/* PROGRAM STATES */
#define WAITING_FOR_HIT			0
#define MEASURING_PEAK			1
#define BATTERY_LOW					2

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

void ADC_Conf(void);
void DMA_Conf(void);
void DisplayBatteryLevel(void);
void DisplayFreq(uint16_t val, char* noteTab);
void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);

float32_t GetBattLvl(void);
float32_t ConvertToDecibels(float32_t amp);
int FindPeak(q15_t* p);
int GetNoteInfo(uint16_t peak, uint8_t* hzIndex, uint8_t* noteIndex);
uint32_t BinSearch(uint16_t t[], uint16_t val, uint16_t length);

#ifdef __UNIT_TESTING
extern q15_t 	input[LENGTH_SAMPLES];
extern q15_t 	output[LENGTH_SAMPLES * 2]; 
#endif

#endif //__MAIN_H
