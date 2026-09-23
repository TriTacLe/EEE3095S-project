/* The only file that knows about the board. Generate the CubeIDE project for
 * the STM32F051 on the course board with the USART that the STM32USB2UART
 * adapter reaches, asynchronous, 115200 8N1, no flow control, and confirm the
 * TX and RX pins against the board pinout before generating code.
 *
 * In the generated main.c, inside the USER CODE markers:
 *
 *     app_init();
 *     HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
 *     while (1) { app_poll(); }
 *
 * The handle name follows whatever CubeIDE generated. huart1 here.
 */
#include <string.h>
#include <string.h>
#include "main.h"          /* CubeIDE: pin macros and the UART handle */
#include "app.h"
#include "port.h"

extern UART_HandleTypeDef huart1;

/* Confirm these two against the course board before the first run. They are
 * the button and LED used in the practicals. */
#define BTN_PORT    GPIOA
#define BTN_PIN     GPIO_PIN_0      /* SW0, active low */
#define LED_PORT    GPIOB
#define LED_PIN     GPIO_PIN_11     /* green */

static uint8_t rx_byte;

void port_tx(const char *line)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)line, strlen(line), 100u);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\n", 1u, 10u);
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

/* One byte at a time keeps the parser fed without a DMA buffer to size. */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart1.Instance)
    {
        app_rx_byte((char)rx_byte);
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1u);
    }
}
