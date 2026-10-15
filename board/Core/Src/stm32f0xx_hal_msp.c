/* HAL infrastructure only. The USART is configured from registers in
 * port_stm32.c, so there is no peripheral MSP callback here. */
#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}
