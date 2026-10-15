#ifndef GAMESERVER_STM32F0XX_IT_H
#define GAMESERVER_STM32F0XX_IT_H

#ifdef __cplusplus
extern "C" {
#endif

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void USART1_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* GAMESERVER_STM32F0XX_IT_H */
