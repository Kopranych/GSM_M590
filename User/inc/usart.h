#ifndef USART_H_
#define USART_H_

#include "stm32f4xx.h"


#define USART_BUS GPIOC
#define TX 				GPIO_Pin_10
#define RX 				GPIO_Pin_11	


void usart_init(void);
void usart_dma_ini(void);
void usart_txstr(char* str);
#endif

