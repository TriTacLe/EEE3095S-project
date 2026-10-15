/* The only file that knows about the board.
 *
 * GPIO goes through the HAL because the GPIO driver ships with the course
 * project. The USART does not: the HAL that comes with the prac projects has no
 * UART driver at all, so USART1 is brought up from registers against RM0091,
 * which is also how the practicals drive their peripherals.
 *
 * USART1 is PA9 for TX and PA10 for RX on AF1. On the current UCT dev board
 * both are wired on-board to the ST-LINK, which doubles as a USB virtual COM
 * port, so the cable used for flashing also carries the serial link and no UART
 * adapter is needed (course announcement, 5 October 2026).
 */
#include <string.h>
#include "main.h"          /* pin macros and stm32f0xx_hal.h */
#include "app.h"
#include "port.h"

/* 48 MHz PCLK over 115200 baud, oversampling by 16, so BRR is the divisor
 * itself on this part. 417 gives 115107 baud, which is 0.08 percent low. */
#define UART_BRR    417u

void port_uart_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9 and PA10 to alternate function, AF1 is USART1 on both. */
    GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
    GPIOA->MODER |=  (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
    GPIOA->AFR[1] &= ~((0xFu << ((9u - 8u) * 4u)) | (0xFu << ((10u - 8u) * 4u)));
    GPIOA->AFR[1] |=  ((0x1u << ((9u - 8u) * 4u)) | (0x1u << ((10u - 8u) * 4u)));

    /* Baud rate must be set while the USART is disabled. 8N1 is the reset
     * state of CR1 and CR2, so only the enables are written here. */
    USART1->CR1 = 0u;
    USART1->BRR = UART_BRR;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

    NVIC_SetPriority(USART1_IRQn, 1u);
    NVIC_EnableIRQ(USART1_IRQn);
}

static void uart_put(char c)
{
    while ((USART1->ISR & USART_ISR_TXE) == 0u)
    {
    }
    USART1->TDR = (uint8_t)c;
}

void port_tx(const char *line)
{
    size_t i;
    size_t n = strlen(line);

    for (i = 0u; i < n; i++)
    {
        uart_put(line[i]);
    }
    uart_put('\n');
}

void port_led(int on)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int port_button_down(void)
{
    return HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN) == GPIO_PIN_RESET;
}

unsigned port_now_ms(void)
{
    return (unsigned)HAL_GetTick();
}

/* One byte at a time keeps the parser fed without a DMA buffer to size. An
 * overrun is cleared and dropped rather than left to wedge the receiver: the
 * parser already recovers at the next newline. */
void port_uart_irq(void)
{
    if ((USART1->ISR & USART_ISR_ORE) != 0u)
    {
        USART1->ICR = USART_ICR_ORECF;
    }
    if ((USART1->ISR & USART_ISR_RXNE) != 0u)
    {
        app_rx_byte((char)(USART1->RDR & 0xFFu));
    }
}
