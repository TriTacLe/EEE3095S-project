/* EEE3095S project: the Serial Game Server on the STM32F051 course board.
 * The clock and the two GPIOs come up here, the USART comes up in port_stm32.c,
 * and the game itself stays in server/core so it can be tested on a PC.
 */
#include "main.h"
#include "app.h"
#include "port.h"

static void gpio_init(void)
{
    GPIO_InitTypeDef pin = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    pin.Pin  = BTN_PIN;
    pin.Mode = GPIO_MODE_INPUT;
    pin.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BTN_PORT, &pin);

    pin.Pin   = LED_PIN;
    pin.Mode  = GPIO_MODE_OUTPUT_PP;
    pin.Pull  = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &pin);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    gpio_init();
    port_uart_init();
    app_init();

    /* app_poll never blocks, so a bare loop is enough. Received bytes arrive
     * through the USART interrupt rather than being polled for here. */
    while (1)
    {
        app_poll();
    }
}

/* 8 MHz crystal -> PLL x6 -> 48 MHz, undivided AHB and APB, which is also what
 * the USART baud rate divisor in port_stm32.c assumes.
 * HAL_RCC_ClockConfig also resets the SysTick millisecond time base. */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clocks = {0};

    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscillator.HSEState       = RCC_HSE_ON;
    oscillator.PLL.PLLState   = RCC_PLL_ON;
    oscillator.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    oscillator.PLL.PREDIV     = RCC_PREDIV_DIV1;
    oscillator.PLL.PLLMUL     = RCC_PLL_MUL6;
    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) Error_Handler();

    clocks.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
                          | RCC_CLOCKTYPE_PCLK1;
    clocks.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_1) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler();
}
#endif
