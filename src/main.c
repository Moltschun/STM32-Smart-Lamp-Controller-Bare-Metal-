/**
 * @file main.c
 * @brief День 14: Умный светильник (Smart Lamp).
 * Логика: Короткий клик - ВКЛ/ВЫКЛ. Удержание - Диммирование.
 */

#include "sys_core.h"
#include "bsp_lamp.h"

// Константы времени
#define DEBOUNCE_TIME   50  // мс (защита от дребезга)
#define HOLD_TIME       500 // мс (порог удержания)

// Состояния лампы
typedef enum {
    MODE_OFF,
    MODE_ON
} LampState_t;

int main(void) {
    // 1. Инициализация систем
    System_Init(); // 72 МГц + SysTick
    Lamp_Init();   // PWM (PA0) + Button (PB1)

    LampState_t state = MODE_OFF;
    int16_t brightness = 0;
    int16_t dim_direction = 20; // Скорость изменения яркости
    
    // Переменные обработки кнопки
    bool is_handling_press = false;
    uint32_t press_start_tick = 0;

    while (1) {
        // 1. ЛОВИМ СОБЫТИЕ НАЖАТИЯ (Из прерывания EXTI)
        if (btn_pressed_event) {
            btn_pressed_event = false; // Сброс флага
            
            // Если мы еще не обрабатываем нажатие - начинаем
            if (!is_handling_press) {
                is_handling_press = true;
                press_start_tick = Get_Tick(); // Засекаем время старта
            }
        }

        // 2. ОБРАБОТКА ЛОГИКИ ЖЕСТОВ
        if (is_handling_press) {
            uint32_t duration = Get_Tick() - press_start_tick;

            // A. Анти-дребезг
            if (duration < DEBOUNCE_TIME) {
                continue;
            }

            // B. Проверяем текущее физическое состояние кнопки
            if (Lamp_ReadButton()) {
                // --- КНОПКА ЕЩЕ НАЖАТА (HOLD) ---
                
                // Если держим дольше порога -> Диммирование
                if (duration > HOLD_TIME) {
                    state = MODE_ON; // Принудительно включаем
                    
                    // Изменяем яркость
                    brightness += dim_direction;
                    
                    // Логика разворота у границ
                    if (brightness >= 1000) {
                        brightness = 1000;
                        dim_direction = -20; // Едем вниз
                    } 
                    else if (brightness <= 10) {
                        brightness = 10;
                        dim_direction = 20;  // Едем вверх
                    }

                    Lamp_SetBrightness(brightness);
                    Delay_Ms(20); // Регулировка скорости плавности
                }
            } 
            else {
                // --- КНОПКА ОТПУЩЕНА (RELEASE) ---
                is_handling_press = false; // Завершаем сеанс

                // Это был Короткий клик?
                if (duration < HOLD_TIME) {
                    // Тоггл режима
                    if (state == MODE_ON) {
                        state = MODE_OFF;
                        Lamp_SetBrightness(0);
                    } else {
                        state = MODE_ON;
                        // Если яркость была на минимуме, включаем на 100%
                        if (brightness < 10) brightness = 1000; 
                        Lamp_SetBrightness(brightness);
                    }
                } else {
                    // Это было Длинное нажатие.
                    // Инвертируем направление для следующего раза (удобство UX)
                    dim_direction = -dim_direction;
                }
            }
        }
    }
}