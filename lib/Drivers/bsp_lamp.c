#include "bsp_lamp.h"

volatile bool btn_pressed_event = false;

void Lamp_Init(void) {
    // 1. Включаем тактирование
    // TIM2 (APB1)
    // GPIOA (PWM), GPIOB (Btn), AFIO (EXTI) -> APB2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    // --- НАСТРОЙКА КНОПКИ (PB1) ---
    // Mode: Input (00), CNF: Input with Pull-up/down (10) -> 0x8
    GPIOB->CRL &= ~(0xF << 4); // Очистка
    GPIOB->CRL |=  (0x8 << 4); 
    
    // Включаем Pull-Up (подтяжка к 3.3В)
    GPIOB->ODR |= (1 << 1);

    // --- НАСТРОЙКА EXTI (PB1 -> EXTI1) ---
    // AFIO_EXTICR[0] управляет пинами 0..3.
    // Нам нужен PIN 1 (биты 4-7). Пишем 0x1 (Port B).
    AFIO->EXTICR[0] &= ~(0xF << 4);
    AFIO->EXTICR[0] |=  (0x1 << 4);

    // Разрешаем прерывание на линии 1
    EXTI->IMR |= (1 << 1);
    // Срабатывание по спаду (Falling Edge - нажатие)
    EXTI->FTSR |= (1 << 1);
    // Отключаем фронт (Rising Edge)
    EXTI->RTSR &= ~(1 << 1);

    // NVIC
    NVIC_SetPriority(EXTI1_IRQn, 1);
    NVIC_EnableIRQ(EXTI1_IRQn);

    // --- НАСТРОЙКА ШИМ (PA0 - TIM2 CH1) ---
    // Mode: Output 2MHz (10), CNF: AF Push-Pull (10) -> 0xA
    GPIOA->CRL &= ~(0xF << 0);
    GPIOA->CRL |=  (0xA << 0);

    // Time Base (1 кГц)
    // 72 МГц / 72 = 1 МГц
    TIM2->PSC = 72 - 1;
    // 1000 тиков = 1 мс
    TIM2->ARR = 1000 - 1;

    // PWM Mode 1
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (0x6 << 4);
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // Preload

    // Enable Output
    TIM2->CCER |= TIM_CCER_CC1E;

    // Start
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void Lamp_SetBrightness(uint16_t value) {
    if (value > 1000) value = 1000;
    TIM2->CCR1 = value;
}

bool Lamp_ReadButton(void) {
    // Читаем IDR. Бит 1. (Active Low)
    return !((GPIOB->IDR >> 1) & 0x1);
}

// --- INTERRUPT HANDLER ---
void EXTI1_IRQHandler(void) {
    if (EXTI->PR & (1 << 1)) {
        // Очистка флага (записью 1)
        EXTI->PR = (1 << 1);
        // Сигнал приложению
        btn_pressed_event = true;
    }
}