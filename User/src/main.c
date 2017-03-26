#include "stm32f4xx.h"                  // Device header
#include "main.h"
//#include "interrupt.h"
//#include "interrupt.c"
#include "usart.h"
#include "GSM_590.h"
#include <string.h>
#include <stdint.h>

typedef struct{
	uint8_t button_flag;
	uint8_t rx_flag;
	uint8_t tx_flag;
}struct_flag;

struct_flag FLAG;
struct_flag* uk;

typedef struct {
	uint8_t buf_rx[40];
	uint8_t buf_tx[40];
}struct_buffer;

struct_buffer buf;
struct_buffer *p;

typedef struct{
	uint8_t size_tx;
	uint8_t size_rx;
	uint8_t count;
	uint8_t count_data;
	uint16_t time;
}count;


count cc;

typedef struct{
	uint8_t temp_data;
	uint8_t temp_address;
}temperery;

temperery temp;

static uint8_t counter = 0;
uint16_t time = 100;
uint16_t delay_count = 0;

enum mode{
	step1,
	step2,
	step3,
	step4,
	step5	
}gsm;

//void init_delay(void);
//void delay_ms(uint16_t delay_temp); // функция задержки реализованная посредством прерываний SysTick
//void SysTick_Handler(void);// вектор прерывания

void init_delay(void)
{
	SysTick_Config(SystemCoreClock/1000);//инициализация SysTick запуск прерываний раз в 1/1000 сек.
}


void SysTick_Handler(void)// вектор прерывания
{
	if(delay_count>0)
		delay_count--;
	if(time>0)
	time--;	
}

void delay_ms(uint16_t delay_temp) // функция задержки реализованная посредством прерываний SysTick
{
	delay_count = delay_temp;
	while(delay_count);
}


void UART4_IRQHandler()
{
	if(USART_GetITStatus(UART4, USART_IT_TXE))
	{
		USART_ClearITPendingBit(UART4, USART_IT_TXE);

		if(cc.count < cc.size_tx )
		{
			USART_SendData(UART4, buf.buf_tx[cc.count]);
			buf.buf_tx[cc.count] = 0;
			cc.count++;
		}
		else
		{
	//		cc.count = 0;
			USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
		}
	}
	if(USART_GetITStatus(UART4, USART_IT_RXNE))
	{
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);
		
		temp.temp_data = USART_ReceiveData(UART4); 
		
		if((temp.temp_data!= 0x0A)&(temp.temp_data!= 0x0D))//отделяем полезные данные от \r\n
		{
			buf.buf_rx[cc.size_rx++] = temp.temp_data;
		}
		if(cc.size_rx > temp.temp_address)// пропускаем первые два символа, и вообще все ненужные символы идущие перед полезной информацией
		{
			if(temp.temp_data == 0x0A)
			{
				buf.buf_rx[cc.size_rx++] = 0x24;// вставляем разделительный символ
				temp.temp_address = cc.size_rx;// сохроняем адрес последнего разделительного символа		
				cc.count_data++;// считаем сколько слов получили
				FLAG.rx_flag = 1;
			}
			
		}
	}

}

void EXTI0_IRQHandler()
{
	EXTI_ClearITPendingBit(EXTI_Line0);
	FLAG.button_flag = 1;
}

void clear_buf(uint8_t *buf);
void buf_str(uint8_t *buf, uint8_t *str);
void* init_struct(void* st, uint8_t x, void* u);
void tx_at_gsm(uint8_t* temp);

//-------------------------------------------------------------------
//-------------------------------------------------------------------
int main()
{
	p = &buf;
	uk = &FLAG;
	init_struct(&buf, 0, &buf);
	init_struct(&FLAG, 0, &FLAG);
	init_struct(&cc, 0, &cc);
	init_struct(&temp, 0, &temp);
	init_delay();
	init_perif();
	usart_init();
//	usart_dma_ini();
	cc.count = 1;
	
		
	
	while(1)
	{
		switch(gsm)
		{
			case step1:
				GPIO_SetBits(GPIOD, PIN_BOOT);		
			case step2:
				if(!time)
				{
					GPIO_ResetBits(GPIOD, PIN_BOOT);
					time = 500;
					cc.size_rx = 0;
					temp.temp_address = 0;
					cc.count_data = 0;
					gsm = step3;
					clear_buf(buf.buf_rx);
				}
				break;
			case step3:
				if(!time)
				{
					if(FLAG.rx_flag)
					{
						if(!strcmp(buf.buf_rx, "MODEM:STARTUP$"))
						{
							GPIO_SetBits(GPIOD, GREEN);
							gsm = step4;
							
						}
						else	gsm = step1;
											
						FLAG.rx_flag = 0;
					}
					else 
					{
						gsm = step1;
						time = 200;
						GPIO_ResetBits(GPIOD, GREEN);
					}
				}
				break;
				
			case step4:
				if(FLAG.rx_flag)
				{
					GPIO_SetBits(GPIOD, BLUE);
					gsm = step5;
					FLAG.rx_flag = 0;
				}
				else 
					{
						gsm = step1;
						time = 200;
						GPIO_ResetBits(GPIOD, BLUE);
					}
				break;
			case step5:
				break;
			default:
				break;
		}
	}
}
		
/*		if(counter == 2)
		{
			__ASM {NOP};
		}
		if(FLAG.button_flag)
		{
			GPIO_ToggleBits(GPIOD, GREEN);
			FLAG.button_flag = 0;
		}
		if(!cc.count)
		{
			tx_at_gsm("AT+CPAS\r");
		}
		if(FLAG.rx_flag)
		{
		
			if(!strcmp(buf.buf_rx, "+PBREADY$"))
		 {
			GPIO_ToggleBits(GPIOD, GREEN);
			 cc.count = 0;
		 }
		 else 
			 if(!strcmp(buf.buf_rx, "+CPAS: 1$"))
			 {
				GPIO_ToggleBits(GPIOD, BLUE);
			 }
			 FLAG.rx_flag = 0;
 	  }
*/		

	


//-------------------------------------------------------------------
//-------------------------------------------------------------------
void buf_str(uint8_t *buf, uint8_t *str)
{
	while(*str&&*buf)
	{
		buf = str;
		buf++;
		str++;
	}
}

void* init_struct(void* st, uint8_t x, void* u)
{
	return memset(st, x, sizeof(u));
}

void tx_at_gsm(uint8_t* temper)
{
	register uint8_t i = 0, * str;
	str = buf.buf_tx;
	do{
		*str++ = temper[i++];
	}while(temper[i] > 0x00);
	cc.size_tx = i;	
	cc.count = 0;
	cc.size_rx = 0;
	temp.temp_address = 0;
	
	USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
}

void clear_buf(uint8_t *bufe)
{
	int t = sizeof(bufe);
	for(int i = 0; i< t; i++)
	{
		bufe[i]=0;
	}
}
