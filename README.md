# STM32 Smart Lamp Controller (Bare Metal)
Проект умного светильника с диммированием, реализованный на микроконтроллере **STM32F103C8T6 (Blue Pill)**. Код написан на **чистом C** с использованием регистров **(CMSIS)**. Основной фокус — реализация неблокирующего управления: одновременная генерация ШИМ сигнала и обработка сложных жестов кнопки (клик/удержание) через прерывания.

![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-STM32F103-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![Architecture](https://img.shields.io/badge/Architecture-ARM_Cortex_M3-0091BD?style=for-the-badge&logo=arm&logoColor=white)
![Standard](https://img.shields.io/badge/Standard-CMSIS-4caf50?style=for-the-badge&logo=arm&logoColor=white)

---

## Hardware Manifest (Аппаратная часть)

* **MCU:** STM32F103C8T6 "Blue Pill"
* **Debug:** ST-Link V2 (SWD)
* **Clock Speed:** 72 MHz (External Crystal 8 MHz + PLL x9)

## Pinout / Схема подключения

| Component | Pin (STM32) | Mode | Примечание |
| :--- | :--- | :--- | :--- |
| **LED (PWM)** | `PA0` | AF Push-Pull | Управление яркостью (TIM2_CH1) |
| **BUTTON** | `PB1` | Input Pull-Up | Кнопка управления (EXTI1) |
| **GREEN LED** | `PA13/PA14` | Debug | Интерфейс отладки |

---

> Важно: Кнопка вынесена на PB1, так как PA0 занят аппаратным таймером `TIM2`.

## Architecture & Drivers (Архитектура)
Проект разделен на уровни абстракции: Ядро (System Core) и Драйвер Устройства (BSP Lamp).

### 1. System Core (RCC & SysTick)
Драйвер переводит микроконтроллер на частоту 72 МГц и запускает системный таймер.

* **`PLL Setup`:** Умножение частоты внешнего кварца (8 MHz x 9).

* **`Bus Prescalers`:** Корректное деление частот для шин APB1 (36 MHz max) и APB2 (72 MHz).

* **`Timebase`:** SysTick настроен на 1 мс для реализации функции `Get_Tick()`.

Справка по регистрам RCC:

* **`RCC_CR`:** Включает HSE (HSEON) и PLL (PLLON). Флаг `PLLRDY` гарантирует стабилизацию частоты.

* **`RCC_CFGR`:** Настраивает делители шин (PPRE1, PPRE2) и множитель `PLL` (PLLMUL9).

* **`FLASH_ACR`:** Задает LATENCY_2 (2 такта ожидания) — критически важно для работы Flash-памяти на 72 МГц.

### 2. PWM Driver (TIM2)
Генерация ШИМ сигнала для плавного управления яркостью светодиода. Используется таймер общего назначения TIM2 (Channel 1).

Frequency: 1 кГц (PSC = 71, ARR = 999).

Resolution: 1000 шагов яркости.

Справка по регистрам TIM:

* **`TIMx_PSC` / `ARR`:** Задают скорость счета (1 мкс) и период цикла (1 мс).

* **`TIMx_CCR1`:** Определяет скважность импульса (яркость). Изменяется динамически в main.

* **`TIMx_CCMR1`:** Бит `OC1PE` (Preload) предотвращает "глюки" сигнала при обновлении яркости на лету.

### 3. Interrupt Driver (EXTI)
Асинхронная обработка нажатия кнопки без постоянного опроса (Polling) в главном цикле.

Маршрутизация: Сигнал с PB1 перенаправляется на линию прерывания EXTI1.

Триггер: Срабатывание по спаду напряжения (Falling Edge), когда кнопка притягивает пин к земле.

Справка по регистрам EXTI:

* **`AFIO_EXTICR`:** "Коммутатор". Выбирает, какой порт (A, B, C) подключен к линии EXTI.

* **`EXTI_IMR`:** "Маска". Разрешает прохождение сигнала прерывания.

* **`EXTI_PR`:** "Флаг". Устанавливается в 1 при событии. Важно: Очищается записью единицы (1), а не нуля.

### 4. Main Logic (Gesture Recognition)
Логика распознавания жестов построена на измерении времени:

Short Click (< 500ms): Переключение ВКЛ/ВЫКЛ.

Long Hold (> 500ms): Вход в режим диммирования. Яркость плавно меняется, пока кнопка нажата.
