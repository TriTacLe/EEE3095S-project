#ifndef APP_H
#define APP_H

void app_init(void);
void app_rx_byte(char c);   /* called from the UART receive interrupt */
void app_poll(void);        /* called from the main loop, must not block */

#endif /* APP_H */
