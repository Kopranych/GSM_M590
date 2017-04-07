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
	uint8_t valid_flag;
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
			cc.count = 0;
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


void reset_gsm(void);
uint8_t inspection_AT(uint8_t* buf_rx, uint8_t* AT);
void clear_buf(uint8_t *buf);
void buf_str(uint8_t *buf, uint8_t *str);
void* init_struct(void* st, uint8_t x, void* u);
void tx_at_gsm(uint8_t* temp);
uint8_t setup_gsm(void);
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
//	cc.count = 1;
	FLAG.rx_flag = 0;
	gsm = step1;
	while(1)
	{	
		switch(gsm){
			case step1:
		reset_gsm();
			time = 20000;
		while(!(inspection_AT(buf.buf_rx, "+PBREADY")))
			{
				if(!time)
					break;
			}
			if(!time)
			{
				gsm = step1;
				GPIO_ResetBits(GPIOD, RED);
				GPIO_ResetBits(GPIOD, BLUE);
			}
			else
			{
				time = 0;
				GPIO_SetBits(GPIOD, RED);
				delay_ms(2000);
				gsm = step2;
				clear_buf(buf.buf_rx);
			}		
		break;
			case step2:
				if(setup_gsm())
				{
					gsm = step3;
					GPIO_SetBits(GPIOD, ORANGE);
				}
				else {
					gsm = step1;
					GPIO_ResetBits(GPIOD, ORANGE);
					GPIO_ResetBits(GPIOD, RED);
					GPIO_ResetBits(GPIOD, BLUE);
				}
				break;
			case step3:
				break;
		}

	}
	
}

		

	


//-------------------------------------------------------------------
//-------------------------------------------------------------------
void reset_gsm(void)
{
	while(!FLAG.valid_flag)
		{
			delay_ms(500);
			tx_at_gsm("AT+CPWROFF\r");
			delay_ms(1000);
			GPIO_SetBits(GPIOD, PIN_BOOT);
			delay_ms(160);
			GPIO_ResetBits(GPIOD, PIN_BOOT);
			delay_ms(1000);
			if(inspection_AT(buf.buf_rx, "MODEM:STARTUP"))
			{
				GPIO_SetBits(GPIOD, BLUE);
				FLAG.valid_flag = 1;
//				clear_buf(buf.buf_rx);
//				cc.count = 0;
//				cc.size_rx = 0;
//				temp.temp_address = 0;
			}
			else 
					GPIO_ResetBits(GPIOD, BLUE);
		}
				
}


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
	cc.count_data = 0;
	USART_ITConfig(UART4, USART_IT_TXE, ENABLE);// Enable interrupt
}


void clear_buf(uint8_t *bufe)
{
	register int t = strlen(bufe);
	for(register int i = 0; i< 40; i++)
	{
		bufe[i]=0;
	}
}



uint8_t inspection_AT(uint8_t* buf_rx, uint8_t* AT)
{
	register uint8_t i = 0, x = 0, b = 0, t = 0,* p;
	p = buf_rx;
	t = strlen(AT);
	b = strlen(buf_rx);
	for(i = 0; i < b+1; i++)
	{	
		if(p[i] != AT[x])
		{
			x = 0;
		}			 

		else
		{
			x++;	
		}	
			if(x == t)
				return 1;
			
	}
	return 0;
}

uint8_t setup_gsm(void)
{
	tx_at_gsm("ATE0\r");// Eho disable
	delay_ms(500);
	if(!(inspection_AT(buf.buf_rx, "OK")))
	{
		return 0;
	}
	clear_buf(buf.buf_rx);
	tx_at_gsm("AT+CLIP=1\r");// Enable AOH
	delay_ms(500);
	while(!(inspection_AT(buf.buf_rx, "OK")))
	{
		return 0;
	}
	clear_buf(buf.buf_rx);
	tx_at_gsm("AT+CMGF=1\r");// text format sms
	delay_ms(500);
	if(!(inspection_AT(buf.buf_rx, "OK")))
	{
		return 0;
	}
	clear_buf(buf.buf_rx);
	return 1;
}