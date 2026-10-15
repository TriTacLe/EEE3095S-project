/* Core exception handlers, the HAL millisecond time base and the USART receive
 * interrupt that feeds the frame parser. */
#include "main.h"
#include "stm32f0xx_it.h"
#include "port.h"

void NMI_Handler(void)
{
    Error_Handler();
}

void HardFault_Handler(void)
{
    Error_Handler();
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void USART1_IRQHandler(void)
{
    port_uart_irq();
}
