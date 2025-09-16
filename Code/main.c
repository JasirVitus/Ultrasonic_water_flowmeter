#include "stm32f0xx.h"
#include "stdio.h"
#include "lcd.h"

GPIO_Config(void);
DMA_Config(void);
ADC_Config(void);
SysTimer_Config(void);
USART_Config(void);
RS_Send(uint8_t ch);

uint32_t tab_adc[50], dane[64], maks = 0, Start_measure = 0, srednia =0, przeplyw = 0;
uint16_t bufor, i, l_pomiary = 0;
uint32_t c;
uint32_t wynik, predkosc, obwod = 227, przelicznik = 11;
int main(void)
{
	SystemInit();

	GPIO_Config();
	DMA_Config();
	ADC_Config();
	SysTimer_Config();
	USART_Config();

    LCD_Init();
    LCD_Clear();
    LCD_Puts((char *)"Wodomierz");


    while(1)
    {
    	if(Start_measure == 0)
    	{
        	asm( "LDR R0, =0x48000014;  \
        	   	   	 LDR R1, =0x00008180;  \
        	   	   	 LDR R2, =0x00008180;  \
        	         LDR R3, =0x00008000;  \ 
        			 LDR R4, =0x00008000;  \
        			 LDR R6, =0x00000000;  \
        			                       \
        			MOV R5, #7 ;           \
        	  start:      		   		   \
        			STR R1, [R0];  		   \
        			NOP;NOP;NOP;NOP;		   \
        						   \
        			STR R2, [R0];  \
        			NOP;NOP;NOP;NOP;NOP;		   \
        			NOP;NOP;NOP;NOP;NOP;		   \
        			NOP;NOP;NOP;NOP;NOP;		   \
        			NOP; \
        						   \
        			STR R3, [R0];  \
        			NOP;NOP;NOP;NOP; \
        						   \
        			STR R4, [R0];  \
        			NOP;NOP;NOP;NOP;NOP;		   \
        			NOP;NOP;NOP;NOP;NOP;		   \
        			NOP;		   \
        						   \
        	    	SUB	R5,	#1 ;   \
        	    	BPL start  ;   \
        	    				   \
        	    	STR R6, [R0];  \
    	    	    				   \
    	    	    				   \
    	    	    				   \
				LDR R0, =0x48000414;  \
				LDR R1, =0x00008000;  \
				LDR R2, =0x00000000;  \
				LDR R3, =0x000004d4;  \
									  \
				Tof:		   \
				SUB	R3,	#1 ;   \
				BPL Tof  ;     \
							   \
				MOV R3, #34;   \
							   \
							   \
				STR R1, [R0];  \
							   \
				ENB_AND: 	   \
				SUB	R3,	#1 ;   \
				BPL ENB_AND ;  \
							   \
				NOP;NOP;       \
				STR R2, [R0];  \
							   \
				");

			ADC1->CR |= ADC_CR_ADSTART;

			while(DMA1->ISR & DMA_ISR_TCIF1 == 0);

			maks = 0;

			for( i=0;i<25; i++)
				{
					if(tab_adc[i]>maks)
					 maks = tab_adc[i];
				}
			ADC1->CR |= ADC_CR_ADSTP;

			DMA1_Channel1->CCR &= ~DMA_CCR_EN;
			DMA1_Channel1->CNDTR = 25;
			DMA1_Channel1->CCR |= DMA_CCR_EN;

			Start_measure = 1;

    	}
    }
}
GPIO_Config(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	// PA0
	        GPIOA->MODER &= ~GPIO_MODER_MODER0_0;
	// PA3 (ADC_MARKER)
			GPIOA->MODER   |= GPIO_MODER_MODER3_0;
			GPIOA->OTYPER  &= ~GPIO_OTYPER_OT_3 ;
			GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3;
			GPIOA->PUPDR   |= GPIO_PUPDR_PUPDR3_1 ;
	// PA8 (TIM1_CH1)
			GPIOA->MODER   |= GPIO_MODER_MODER8_0;
			GPIOA->OTYPER  &= ~GPIO_OTYPER_OT_8 ;
			GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8;
			GPIOA->PUPDR   |= GPIO_PUPDR_PUPDR8_1 ;
	// PA7 (TIM1_CH2)
			GPIOA->MODER   |= GPIO_MODER_MODER7_0;
			GPIOA->OTYPER  &= ~GPIO_OTYPER_OT_7 ;
			GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7;
			GPIOA->PUPDR   |= GPIO_PUPDR_PUPDR7_1 ;
	// PA15 (MUX Switch)
			GPIOA->MODER   |= GPIO_MODER_MODER15_0;
			GPIOA->OTYPER  &= ~GPIO_OTYPER_OT_15 ;
			GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15;
			GPIOA->PUPDR   |= GPIO_PUPDR_PUPDR15_1 ;
	// PB15 (Enable AND)
			GPIOB->MODER   |= GPIO_MODER_MODER15_0;
			GPIOB->OTYPER  &= ~GPIO_OTYPER_OT_15 ;
			GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15;
			GPIOB->PUPDR   |= GPIO_PUPDR_PUPDR15_1 ;
	//PA1 (ADC)
			GPIOA->MODER |=GPIO_MODER_MODER1_0 | GPIO_MODER_MODER1_1; 	  // Analog mode
			GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR1_0); //
	//PA9 USART TX
			GPIOA->MODER  |= GPIO_MODER_MODER9_1;   // ALTERNATE Function
			GPIOA->AFR[1] |= 0x01 << 0x04;   		// USART1_TX
			GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_0; 	// Pull-Up
}

DMA_Config(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	DMA1_Channel1->CPAR = (uint32_t) (&(ADC1->DR)); // Peripherial Adress
	DMA1_Channel1->CMAR = (uint32_t)(tab_adc);    // Memory Adress
	DMA1_Channel1->CNDTR = 25;                     // Number of data to Transfer

	DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_TEIE | DMA_CCR_TCIE | DMA_CCR_MSIZE_1| DMA_CCR_PSIZE_1;
	// Memory Increment | Transfer error Interrupt Enable | Transfer Complete Interrupt Enable
	DMA1_Channel1->CCR |= DMA_CCR_EN; // DMA ENABLE
}

ADC_Config(void)
{
	RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;  // Reset ADC
	RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST; // Release from reset

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;    // ENABLE ADC Clock

	if ((ADC1->CR & ADC_CR_ADEN) != 0)
	{
	 ADC1->CR |= ADC_CR_ADDIS;
	}
	while ((ADC1->CR & ADC_CR_ADEN) != 0)
	{
	}
	ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN;
	ADC1->CR |= ADC_CR_ADCAL;
	while ((ADC1->CR & ADC_CR_ADCAL) != 0)
	{
	}

    ADC1->CFGR1 = 0x00;                // Reset ADC Configuration
	ADC1->CFGR1 |= ADC_CFGR1_CONT | ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG | ADC_CFGR1_RES_1; // Continuous mode | enable DMA | 12 bit RES
	ADC1->CHSELR = ADC_CHSELR_CHSEL1; // CH1

	ADC1->SMPR |= 0x00;             // Sampling time 7.5
	ADC1->IER  |= ADC_IER_EOSIE;    // Interrupt Enable EOS

	ADC1->CR |= ADC_CR_ADEN; // Enable ADC
}
SysTimer_Config(void)
{
	SysTick_Config( SystemCoreClock/10 + 1);

	NVIC_EnableIRQ(SysTick_IRQn);
}
USART_Config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	USART1->CR1  = 0x00; // Reset
	USART1->CR1 |= USART_CR1_TE ; // | (Hardware flow control none) | Transmitter enable
	//USART1->CR2 |=  ;          // (Stop bits: 1)
	USART1->BRR = 480000 / 96;   // 9600 bauds

	USART1->CR1 |= USART_CR1_UE ; // ENABLE USART
}

RS_Send(uint8_t ch)
{
	USART1->TDR = (ch & (uint16_t)0x01FF);
	//USART1->TDR = ch;
	while( (USART1->ISR & USART_ISR_TXE) == 0){}
}

SysTick_Handler()
{
	if(GPIOA->IDR & GPIO_IDR_0 != 0)
	{
		przeplyw  =0;
		l_pomiary =1;
	}
	else {
		srednia += maks;

			if( l_pomiary == 5)
			{
				l_pomiary = 1;

				srednia = srednia /5;

				wynik = 17*srednia*srednia + 6775*srednia - 266170; // y = 0,0017x2 + 0,6775x - 26,617

				wynik = wynik / 10000;

				if(srednia < 40)
					wynik = 0;

				predkosc = wynik * przelicznik/10;
				przeplyw = przeplyw + predkosc * obwod/100;

				LCD_Clear();

				LCD_Puts((char *)"Speed: ");
				LCD_PutUnsignedInt(predkosc);
				LCD_Puts((char *)"cm/s");
				LCD_Goto(1,2);

				LCD_Puts((char *)"Flow:  ");
				LCD_PutUnsignedInt(przeplyw);
				LCD_Puts((char *)"L");
				srednia = 0;

				for( i=0;i<25; i++)
					{
						RS_Send(tab_adc[i]);
						for(c=0;c<16000;c++);
					}
			}
			else
				l_pomiary++;
	}

	Start_measure = 0;
}

