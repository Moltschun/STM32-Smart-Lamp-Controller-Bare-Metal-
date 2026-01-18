#ifndef BSP_LAMP_H_
#define BSP_LAMP_H_

#include "stm32f1xx.h"
#include <stdbool.h>

// Глобальный флаг события (устанавливается в прерывании)
extern volatile bool btn_pressed_event;

void Lamp_Init(void);
void Lamp_SetBrightness(uint16_t value); // 0-1000
bool Lamp_ReadButton(void);

#endif