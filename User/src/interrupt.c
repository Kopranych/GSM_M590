
#include "interrupt.h"

uint16_t delay_count = 0;

void init_delay(void)
{
	SysTick_Config(SystemCoreClock/1000);//инициализация SysTick запуск прерываний раз в 1/1000 сек.
}


void SysTick_Handler(void)// вектор прерывания
{
	if(delay_count>0)
		delay_count--;
}

void delay_ms(uint16_t delay_temp) // функция задержки реализованная посредством прерываний SysTick
{
	delay_count = delay_temp;
	while(delay_count);
}


