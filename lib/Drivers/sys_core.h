/**
 * @file sys_core.h
 * @brief Системное ядро: Настройка тактирования (RCC) и Времени (SysTick).
 * @version 1.0
 */

#ifndef SYS_CORE_H_
#define SYS_CORE_H_

#include "stm32f1xx.h"

// Глобальный счетчик миллисекунд (инкрементируется в прерывании)
extern volatile uint32_t ms_ticks;

/**
 * @brief Инициализация системы.
 * 1. Настройка Flash Latency.
 * 2. Настройка PLL (HSE 8MHz * 9 = 72MHz).
 * 3. Настройка SysTick (1 мс).
 */
void System_Init(void);

/**
 * @brief Получить текущее время системы в мс.
 * @return Количество мс с момента старта.
 */
uint32_t Get_Tick(void);

/**
 * @brief Блокирующая задержка.
 * @param ms Количество миллисекунд.
 */
void Delay_Ms(uint32_t ms);

#endif /* SYS_CORE_H_ */