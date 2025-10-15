#include "nvic.h"
#include "gpio.h"
#include "room_control.h"
#include "uart.h"

extern volatile uint32_t ms_counter;

// ------------------------------
// Habilita EXTI13 (PC13) en NVIC
// ------------------------------
void nvic_exti_pc13_button_enable(void)
{
    // SYSCFG clock para enrutar EXTI
    RCC->APB2ENR |= (1U << 0);      // SYSCFG

    // PC13 -> EXTI13: EXTICR4 bits para la línea 13 (C = 0b10)
    SYSCFG->EXTICR[3] &= ~(0xFU << 4);
    SYSCFG->EXTICR[3] |=  (0x2U << 4);

    // Desenmascara la línea 13
    EXTI->IMR1 |= (1U << 13);
    // Flanco descendente (botón activo-bajo en Nucleo)
    EXTI->FTSR1 |= (1U << 13);
    // Limpia pendiente
    EXTI->PR1 = (1U << 13);

    // Habilita IRQ en NVIC (EXTI15_10)
    NVIC->ISER[1] = (1U << (EXTI15_10_IRQn - 32));
}

// ---------------------------------------
// Habilita interrupción de RX en USART2
// ---------------------------------------
void nvic_usart2_irq_enable(void)
{
    // Habilita RXNEIE (interrupción cuando hay dato)
    USART2->CR1 |= (1U << 5);

    // Habilita IRQ de USART2 en NVIC
    NVIC->ISER[1] = (1U << (USART2_IRQn - 32));
}
