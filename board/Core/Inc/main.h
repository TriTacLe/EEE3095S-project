#ifndef GAMESERVER_MAIN_H
#define GAMESERVER_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"

/* The button and the LED. Taken from the Practical 1A and 1B projects that ran
 * on this board: buttons sit on PA0 to PA3 behind pull-ups and are active low,
 * LEDs sit on PB0 to PB7. */
#define BTN_PORT    GPIOA
#define BTN_PIN     GPIO_PIN_0      /* Button0, active low */
#define LED_PORT    GPIOB
#define LED_PIN     GPIO_PIN_0      /* LED0 */

void SystemClock_Config(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* GAMESERVER_MAIN_H */
