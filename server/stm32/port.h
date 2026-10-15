/* What the application needs from the board. The board side implements these
 * and nothing else, which is why app.c has no HAL in it. */
#ifndef PORT_H
#define PORT_H

#include <stddef.h>

void     port_uart_init(void);        /* main() calls this before app_init */
void     port_uart_irq(void);         /* the USART interrupt calls this */

void     port_tx(const char *line);   /* send the line and a '\n' */
void     port_led(int on);
int      port_button_down(void);      /* 1 while the button is held */
unsigned port_now_ms(void);

#endif /* PORT_H */
