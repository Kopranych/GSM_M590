#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "interrupt.h"
#include "usart.h"
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
	uint8_t buf_rx[16];
	uint8_t buf_tx[16];
}struct_buffer;

struct_buffer buf;
struct_buffer *p;

typedef struct{
	uint8_t size_tx;
	uint8_t size_rx;
	uint8_t count;
}count;

count cc;

//uint8_t u[10] = "GREEN";

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
		if(USART_ReceiveData(UART4)!= 0x0D)
		{
			buf.buf_rx[cc.size_rx] = USART_ReceiveData(UART4);
			cc.size_rx++;
		}
		else {
			buf.buf_rx[cc.size_rx] = 0x00;
			FLAG.rx_flag = 1;
			cc.size_rx = 0;
		}
	}
}

void EXTI0_IRQHandler()
{
	EXTI_ClearITPendingBit(EXTI_Line0);
	FLAG.button_flag = 1;
}


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
	init_delay();
	init_perif();
	usart_init();
//	usart_dma_ini();
//	cc.count = 1;
	while(1)
	{
		if(FLAG.button_flag)
		{
			GPIO_ToggleBits(GPIOD, GREEN);
			FLAG.button_flag = 0;
		}
		if(!cc.count)
		{
			tx_at_gsm("AT\r");
		}
		if(FLAG.rx_flag)
		{
		 if(!strcmp(buf.buf_rx, "GREEN"))
		 {
			GPIO_ToggleBits(GPIOD, GREEN);
		 }
		 else 
			 if(!strcmp(buf.buf_rx, "BLUE"))
			 {
				GPIO_ToggleBits(GPIOD, BLUE);
			 }
			 FLAG.rx_flag = 0;
 	  }
		
	}
}
	
	


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

void tx_at_gsm(uint8_t* temp)
{
	register uint8_t i = 0, * str;
	str = buf.buf_tx;
	do{
		*str++ = temp[i++];
	}while(temp[i] > 0x00);
	cc.size_tx = i;	
	cc.count = 0;
	USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
}
