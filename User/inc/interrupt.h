#ifndef INTERRUPT_H_
#define INTERRUPT_H_


#include "stm32f4xx.h"                  // Device header

/*extern uint16_t time;
uint16_t delay_count = 0;

void init_delay(void);
void delay_ms(uint16_t delay_temp); // функция задержки реализованная посредством прерываний SysTick
void SysTick_Handler(void);// вектор прерывания





void init_delay(void)
{
	SysTick_Config(SystemCoreClock/1000);//инициализация SysTick запуск прерываний раз в 1/1000 сек.
}


void SysTick_Handler(void)// вектор прерывания
{
	if(delay_count>0)
		delay_count--;
	time++;	
}

void delay_ms(uint16_t delay_temp) // функция задержки реализованная посредством прерываний SysTick
{
	delay_count = delay_temp;
	while(delay_count);
}
*/
#endif /*INTERRUPT_H_*/
