/* Includes ------------------------------------------------------------------*/

#ifdef __UNIT_TESTING
#include "minunit.h"
#endif

#include "main.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */
	
/* Private variables ---------------------------------------------------------*/

uint32_t  ifftFlag = 0; 
uint32_t  doBitReverse = 1;
uint32_t  sampleCounter = 0;
float32_t adcMicVal = 0.0f;

ADC_InitTypeDef 	ADC_InitStructure;
DMA_InitTypeDef 	DMA_InitStructure;
GPIO_InitTypeDef 	GPIO_InitStructure;

__IO uint32_t isSampleTaken = 0;
__IO uint32_t TimingDelay;
__IO uint16_t ADCVal[2];

uint16_t 	peak = 0;
uint8_t 	result = 0;
uint8_t 	hzIndex, noteIndex;
uint8_t 	state = 0;

q15_t 	input[LENGTH_SAMPLES];
q15_t 	output[LENGTH_SAMPLES * 2];  // [60], [65, 66, 67, 68, 69], [1000]

uint16_t tabHz[] = { 65,  69,  73,  78,  82,  87,  93,  98, 104, 110, 117, 123,
										131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247,
										262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
										523, 554, 587, 622, 659, 699, 740, 784, 813, 880, 932, 988 };

char* tabNote[] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "H " };

/* Private functions ---------------------------------------------------------*/
	
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */	
	
int main(void)
{
	
#ifndef __UNIT_TESTING
	
	arm_status status; 
  arm_rfft_instance_q15 rfft; 
  arm_cfft_radix4_instance_q15 cfft;  
	
  status = ARM_MATH_SUCCESS; 
  status = arm_rfft_init_q15(&rfft, &cfft, FFT_SIZE, ifftFlag, doBitReverse);	

	if( status != ARM_MATH_SUCCESS){ while (1); } 

	/* ADCCLK = PCLK2/4 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);
	
	/* Enable peripheral clocks ------------------------------------------------*/
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 and GPIO clocks --------------------------------------------*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO/*?*/ | 
		RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	/* Disable the Serial Wire Jtag Debug Port SWJ-DP */
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	
	/* Configure LCD pins -----------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = LCD_RS | LCD_E | LCD_D4 | LCD_D5 | LCD_D7 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Configure PB.1 (LED Blink) and LCD (D6, RW) pins -------------------------*/
	GPIO_InitStructure.GPIO_Pin = /*GPIO_Pin_1 |*/ LCD_D6 | LCD_RW;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure PA.0 (ADC1 Channel0) as analog input -------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;// | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Configure PB.0 (ADC1 Channel8) as analog input -------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	DMA_Conf();
	ADC_Conf();
	
	if (SysTick_Config(SystemCoreClock / 2048)){ while (1); }
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);
	Delay(100);	
	LCD_Init();

	//if(GetBattLvl() > BATTERY_LOW_LEVEL)
	{
		LCD_Clear();
		LCD_WriteString("Drum2na", 7);
		LCD_GotoXY(0, 1);
		LCD_WriteString("Welcome", 7);
		Delay(4000);	
		DisplayBatteryLevel();
		Delay(4000);	
		LCD_Clear();
		LCD_WriteString("Drum2na", 7);
		LCD_GotoXY(0, 1);
		LCD_WriteString("Hit!", 7);
	}
	
	printf("Hi there!");
	
  while (1)
  {
		/*if(GetBattLvl() < BATTERY_LOW_LEVEL)
		{
			LCD_Clear();
			Delay(1000);
			LCD_WriteString("Low", 3);
			LCD_GotoXY(0, 1);
			LCD_WriteString("Battery!", 8);
			Delay(1000);
		}
		else */if(isSampleTaken == 1) /* Mic sample taken */
		{
			adcMicVal = ADCVal[0] - 0x7FF;
			adcMicVal /= 0x7FF;
			
			switch(state)
			{
				
				case WAITING_FOR_HIT:
				{
					if(ConvertToDecibels(adcMicVal) > TRIGGER_AT_DB) /* Trigger value */
					{
						state = MEASURING_PEAK;
						GPIO_SetBits(GPIOA, GPIO_Pin_15);
					}
					break;
				}
				
				case MEASURING_PEAK:
				{
					
					/* Convert adcMicVal from float32_t to q15 format */
					arm_float_to_q15(&adcMicVal, &input[sampleCounter++], 1);

					if(sampleCounter >= LENGTH_SAMPLES)
					{				
						
						/* ----------------------------------------------------------------------------- */
						/* ----------------------------------------------------------------------------- */
						/* Calculate FFT */
						arm_rfft_q15(&rfft, input, output); 
						
						peak = FindPeak(output);
						
						//precisePeak = FindPeakDouble(output, FFT_SIZE, peak);

						/* Display measured frequancy */
						LCD_Clear();
						LCD_Home();		
						
						//DisplayFreqValue(maxIndex, NULL); /* wyswietlanie liczb z przecinkami */
						DisplayFreq(peak, NULL);
						
						/* ----------------------------------------------------------------------------- */
						/* Finding neastest note that represents measured frequancy                      */
						/* ----------------------------------------------------------------------------- */
						result = GetNoteInfo(peak, &hzIndex, &noteIndex);
						
						switch(result)
						{
							case 0: {
												LCD_GotoXY(0, 1);
												DisplayFreq(tabHz[hzIndex], tabNote[noteIndex]); 
												break;
											}
							
							case 1: {
												LCD_GotoXY(0, 1);
												LCD_WriteString("high!", 5);
												break;
											}
											
							case 2: {
												LCD_GotoXY(0, 1);
												LCD_WriteString("low!", 4);
												break;
											}
											
							default:break;
						}
						/* ----------------------------------------------------------------------------- */
						/* ----------------------------------------------------------------------------- */
						
						/* Turn off LED - measuring is over*/
						GPIO_ResetBits(GPIOA, GPIO_Pin_15);
						/* Set state to wait for hit */
						state = WAITING_FOR_HIT;						
						sampleCounter = 0;
						
					}
					
					isSampleTaken = 0;
					
				}
			}
			
		}
	}
	
#else 
	
	test_run();
	while(1);
	
#endif
}

/**
  * @brief  ADC configuration
  * @param  None
  * @retval None
  */

void ADC_Conf(void)
{
	ADC_InitTypeDef ADC_InitStructure; 

  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 
  ADC_InitStructure.ADC_ScanConvMode = ENABLE; 
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
  ADC_InitStructure.ADC_NbrOfChannel = 2; 
  ADC_Init(ADC1, &ADC_InitStructure); 

  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_55Cycles5); 

  ADC_DMACmd(ADC1, ENABLE); 
  ADC_Cmd(ADC1, ENABLE); 

  // kalibracja 
  ADC_ResetCalibration(ADC1); 
  while(ADC_GetResetCalibrationStatus(ADC1)); 

  ADC_StartCalibration(ADC1); 
  while(ADC_GetCalibrationStatus(ADC1)); 

  ADC_SoftwareStartConvCmd(ADC1,ENABLE); 
}

/**
  * @brief  DAC configuration.
  * @param  None
  * @retval None
  */

void DMA_Conf(void) 
{ 
   DMA_InitTypeDef DMA_InitStruct; 

   DMA_DeInit(DMA1_Channel1); 
   DMA_InitStruct.DMA_PeripheralBaseAddr = ADC1_DR_Address; 
   DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&ADCVal; 

   // kierunek: zrod³o to ADC 
   DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC; 

   // rozmiar buffora u nas 2 
   DMA_InitStruct.DMA_BufferSize = 2; 

   DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
   DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable; 
   DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
   DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 

   //ciag³e przesy³anie danych 
   DMA_InitStruct.DMA_Mode = DMA_Mode_Circular; 

   DMA_InitStruct.DMA_Priority = DMA_Priority_High; 
   DMA_InitStruct.DMA_M2M = DMA_M2M_Disable; 

   DMA_Init(DMA1_Channel1, &DMA_InitStruct); 

   //wlacz DMA 
   DMA_Cmd(DMA1_Channel1, ENABLE); 
} 

/**
  * @brief  Gets note based on the peak frequancy value.
  * @param  None
  * @retval None
  */

int GetNoteInfo(uint16_t peak, uint8_t* hzIndex, uint8_t* noteIndex)
{
	int16_t j, k, diff0, diff1;
	
	j = BinSearch(tabHz, peak, 48);
	k = j - (uint32_t)(j / 12) * 12;

	if( (peak >= tabHz[0]) && (peak <= tabHz[47]))
	{
		diff0 = tabHz[j] - peak;
		if(j <= 0)diff1 = peak - tabHz[0];
		else diff1 = peak - tabHz[j - 1];
							
		//LCD_GotoXY(0, 1);
							
		if(diff1 > diff0){				
			//DisplayFreqValue(tabHz[j], tabNote[k]);
			*hzIndex = j;
			*noteIndex = k;
		}
		else						
		{
			if(j <= 0)
			{
				//DisplayFreqValue(tabHz[0], tabNote[0]);
				*hzIndex = 0;
				*noteIndex = 0;
			}				
			//a co dla k > 48?
			else {
				if(k - 1 < 0){ *hzIndex = j - 1; *noteIndex = 0;/*DisplayFreqValue(tabHz[j - 1], tabNote[0]);*/ }
				else { *hzIndex = j - 1; *noteIndex = k - 1; /*DisplayFreqValue(tabHz[j - 1], tabNote[k - 1]);*/ }
			}
		}								
	}
	else if(peak > tabHz[47] )
	{
		//LCD_GotoXY(0, 1);
		//LCD_WriteString("high!", 5);
		return 1;
	}
	else if(peak < tabHz[0] )
	{
		//LCD_GotoXY(0, 1);
		//LCD_WriteString("low!", 4);
		return 2;
	}
	
	return 0;
}

/**
  * @brief  Finds peak frequancy in pointed table.
  * @param  None
  * @retval None
  */

int FindPeak(q15_t* p)
{
	float32_t realIn, imagIn, peak;
	q15_t maxValue = 1.0f;
	uint16_t i, maxIndex = 0;
	
	for(i = 40; i < 2000; i += 2) /* Starting from 20Hz */
	{
		//zastosuj filtr od 0 do 65Hz
		realIn = p[i] >> 5; /* Konwersja q15 na f32 */
		imagIn = p[i+1] >> 5;
								
		arm_sqrt_f32((realIn * realIn) + (imagIn * imagIn), &peak);						
		output[i] = (q15_t)(10 * log10(peak));	 			
		
		/* Filtr górnoprzepustowy -6.02dB do 65Hz*/
#ifndef __UNIT_TESTING
		if(i < 130 || i > 512)p[i] -= 8; 
#endif	
		
		if(maxValue < p[i])
		{
			maxValue = p[i];							
			maxIndex = i/2;
		}				
	
	}
	
	return maxIndex;
}

void DisplayFreq(uint16_t val, char* noteTab)
{
	LCD_WriteNumber(val);
	LCD_GotoXY(6, 1);
	if(noteTab != NULL){
		LCD_WriteString(noteTab, 2);
	}
	LCD_GotoXY(6, 0);
	LCD_WriteString("Hz", 2);
}

/**
  * @brief  Converts amplitude to decibels.
  * @param  amp: amplitude of signal.
  * @retval None
  */

float32_t ConvertToDecibels(float32_t amp)
{
	float32_t dec = -80.0f; 		 /* Begin with silence */
	if(amp == 0.0f)return dec;   /* Return silence     */
	if(amp < 0.0f) amp *= -1.0f; /* ABS                */
	
	dec = 20 * log10(amp);			 /* Calculate dB       */ 
	return dec;
}

void DisplayBatteryLevel(void)
{
	float32_t bat;
	int i = 0;
	LCD_Clear();
	LCD_WriteString("Battery", 7);
	LCD_GotoXY(0, 1);
	
	bat = GetBattLvl();
	
	for(i=0;i<(int)bat;i++)
		LCD_WriteData(0xff);	
	
	for(;i<8;i++)
		LCD_WriteData('-');	
}

float32_t GetBattLvl(void)
{
	float32_t bat;
	/* Zmienna bat zawiera % wartosc naladowania baterii 3.3V to napiecie 
	   zasilania, 2.87 to napiecie z dzielnika napieciowego baterii 9V, 
	   8 to liczba segmentow wyswietlacza LCD, 1.5 to wartosc przy której 
	   bateria 9V jest uznawana za rozladowana */

	bat = (((3.3f *  ADCVal[1] / 4096.0f) - 1.5f) / (2.87f - 1.5f)) * 8.0f;
	return bat;
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
    TimingDelay--;
	isSampleTaken = 1;
}

void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;
  while(TimingDelay != 0);
}

uint32_t BinSearch(uint16_t t[], uint16_t val, uint16_t length)
{
    uint16_t p = 0, k = length, c = 0;
    while(p!=k){
        c = (p+k)/2;
        if(c == 0 && t[c]>val)return 0;
        if((t[c]>val && t[c-1]<val) ||
            t[c]==val){
                return c;
        }
        else{
            if( val < t[c] )
                k = c - 1;
            else
                p = c + 1;
        }
    }
    return k;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


