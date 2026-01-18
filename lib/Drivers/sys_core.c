/**
 * @file sys_core.c
 * @brief Реализация системных функций (CMSIS).
 */

#include "sys_core.h"

// Переменная для хранения времени
volatile uint32_t ms_ticks = 0;

// Прототипы локальных функций
static void RCC_Init(void);
static void SysTick_Init(void);

// --- PUBLIC FUNCTIONS ---

void System_Init(void) {
    RCC_Init();     // Разгон до 72 МГц
    SysTick_Init(); // Запуск таймера времени
}

uint32_t Get_Tick(void) {
    return ms_ticks;
}

void Delay_Ms(uint32_t ms) {
    uint32_t start_time = ms_ticks;
    // Ожидаем, пока разница между текущим и стартовым временем не станет больше ms
    // (Корректно работает даже при переполнении uint32_t)
    while ((ms_ticks - start_time) < ms);
}

// --- INTERRUPT HANDLERS ---

/**
 * @brief Обработчик системного таймера (вызывается каждые 1 мс).
 */
void SysTick_Handler(void) {
    ms_ticks++;
}

// --- PRIVATE FUNCTIONS ---

static void SysTick_Init(void) {
    // Формула: Clock / Частота_прерываний
    // 72 000 000 / 1000 = 72 000 тиков
    SysTick->LOAD = 72000 - 1; 
    SysTick->VAL = 0; // Сброс текущего значения
    
    // CTRL:
    // CLKSOURCE (Bit 2) = 1 (Processor Clock 72MHz)
    // TICKINT   (Bit 1) = 1 (Разрешить прерывания)
    // ENABLE    (Bit 0) = 1 (Включить таймер)
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
                    SysTick_CTRL_TICKINT_Msk   | 
                    SysTick_CTRL_ENABLE_Msk;
}

static void RCC_Init(void) {
    // 1. Включаем HSE (внешний кварц)
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)); // Ждем готовности

    // 2. Настройка Flash (Latency)
    // Для 72 МГц нужно 2 цикла задержки
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    // 3. Настройка делителей шин
    // HCLK (AHB)  = SYSCLK (72 MHz) -> DIV1
    // PCLK2 (APB2) = HCLK (72 MHz)   -> DIV1
    // PCLK1 (APB1) = HCLK / 2 (36 MHz) -> DIV2 (Макс для APB1 = 36 МГц!)
    RCC->CFGR |= (RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_HPRE_DIV1);

    // 4. Настройка PLL
    // Источник: HSE, Множитель: 9 (8 * 9 = 72)
    RCC->CFGR &= ~(RCC_CFGR_PLLMULL | RCC_CFGR_PLLSRC); // Очистка
    RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);

    // 5. Включаем PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)); // Ждем разгона

    // 6. Переключаем системное тактирование на PLL
    RCC->CFGR &= ~RCC_CFGR_SW;      // Очистка битов SW
    RCC->CFGR |= RCC_CFGR_SW_PLL;   // Выбор PLL
    
    // 7. Ждем, пока процессор подтвердит переключение
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}