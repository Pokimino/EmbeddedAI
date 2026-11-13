#ifndef APP_AIFES_H
#define APP_AIFES_H
#include <stdint.h>
void APP_AIFES_Init(void);
void APP_AIFES_Process(void);
/* Inspect in debugger if needed; no text is sent on the binary UART. */
extern volatile uint32_t aifes_last_error;
extern volatile uint32_t aifes_frames_ok;
#endif
