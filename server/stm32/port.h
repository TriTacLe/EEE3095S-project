/* What the application needs from the board. The CubeIDE side implements
 * these three and nothing else, which is why app.c has no HAL in it. */
#ifndef PORT_H
#define PORT_H

#include <stddef.h>

void     port_tx(const char *line);   /* send the line and a '\n' */
void     port_led(int on);
int      port_button_down(void);      /* 1 while the button is held */
unsigned port_now_ms(void);

#endif /* PORT_H */
