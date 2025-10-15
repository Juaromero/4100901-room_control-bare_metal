#include "rcc.h"

void rcc_init(void)
{
    RCC->AHB2ENR |= (1 << 0);                      // Habilita reloj GPIOA
    RCC->AHB2ENR |= (1 << 2);                      // Habilita reloj GPIOC
}
rcc_syscfg_clock_enable();

void rcc_tim3_clock_enable(void)
{
    // TIM3 está en APB1ENR1 bit 1 en L476
    RCC->APB1ENR1 |= (1U << 1);
}
