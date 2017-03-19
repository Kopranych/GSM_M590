#include "usart.h"


typedef struct{
	uint8_t buf_rx[16];
	uint8_t buf_tx[16];
}struct_buffer;

extern struct_buffer buf;


//---------------------------------------------------------------------------------
void usart_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	
	GPIO_InitTypeDef usart_bus;
	USART_InitTypeDef usart1;
	
	usart_bus.GPIO_Pin = TX|RX;
	usart_bus.GPIO_Mode = GPIO_Mode_AF;
	usart_bus.GPIO_Speed = GPIO_Speed_50MHz;
	usart_bus.GPIO_OType = GPIO_OType_PP;
	usart_bus.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_Init(USART_BUS, &usart_bus);
	GPIO_PinAFConfig(USART_BUS, GPIO_PinSource10, GPIO_AF_UART4);
	GPIO_PinAFConfig(USART_BUS, GPIO_PinSource11, GPIO_AF_UART4);
	
	usart1.USART_BaudRate = 9600;//
	usart1.USART_WordLength = USART_WordLength_8b;//
	usart1.USART_StopBits = USART_StopBits_1;
	usart1.USART_Parity = USART_Parity_No;//бит четности
	usart1.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//аппаратный контроль потока RTS CTS

	USART_Init(UART4, &usart1);
	USART_Cmd(UART4, ENABLE);
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);// локальные прерывания
	NVIC_EnableIRQ(UART4_IRQn);//разрешаем обшие прерывания
	
}
//---------------------------------------------------------------------------------
void usart_dma_ini(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	//TX
	DMA_InitTypeDef dma_usart;
	dma_usart.DMA_Channel = DMA_Channel_4;
	dma_usart.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	dma_usart.DMA_Memory0BaseAddr = (uint32_t)buf.buf_tx;
	dma_usart.DMA_DIR =DMA_DIR_MemoryToPeripheral;
	dma_usart.DMA_BufferSize = sizeof(buf.buf_tx);
	dma_usart.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_usart.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_usart.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma_usart.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma_usart.DMA_Mode = DMA_Mode_Normal;
	dma_usart.DMA_Priority = DMA_Priority_Medium;
	dma_usart.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_usart.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_usart.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_usart.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(DMA1_Stream4, &dma_usart);
	//DMA_Cmd(DMA1_Stream4, ENABLE);
	
	NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
	//USART_DMACmd(UART4, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);
	
	//RX
	dma_usart.DMA_Channel = DMA_Channel_4;
	dma_usart.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	dma_usart.DMA_Memory0BaseAddr = (uint32_t)buf.buf_rx;
	dma_usart.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma_usart.DMA_BufferSize = sizeof(buf.buf_rx);
	dma_usart.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_usart.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_usart.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma_usart.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma_usart.DMA_Mode = DMA_Mode_Normal;
	dma_usart.DMA_Priority = DMA_Priority_Medium;
	dma_usart.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_usart.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_usart.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_usart.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(DMA1_Stream2, &dma_usart);
	DMA_Cmd(DMA1_Stream2, ENABLE);
	
	NVIC_EnableIRQ(DMA1_Stream2_IRQn);
	DMA_ITConfig(DMA1_Stream2, DMA_IT_TC, ENABLE);
	//USART_DMACmd(UART4, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);
}
//---------------------------------------------------------------------------------
void usart_txstr(char* str)
{
	while(*str)
	{
		while(!(USART_GetFlagStatus(UART4, USART_FLAG_TC)));
		USART_SendData(UART4, *str);	
		str++;
	}
}
//---------------------------------------------------------------------------------





