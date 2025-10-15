#include "gpio.h" 


void init_gpio(GPIO_Typedef_t *GPIO, uint8_t pin, uint8_t mode, uint8_t type, uint8_t speed, uint8_t pupd, uint8_t initial_value){
    GPIO->MODER &= ~(3 << (pin*2));           // Limpia bits
    GPIO->MODER |=  (mode << (pin*2));        // Configura modo

    GPIO->TYPER &= ~(1 << pin);                // Limpia bit
    GPIO->TYPER |=  (type << pin);             // Configura tipo

    GPIO->SPEEDR &= ~(3 << (pin*2));          // Limpia bits
    GPIO->SPEEDR |=  (speed << (pin*2));      // Configura velocidad

    GPIO->PUPDR &= ~(3 << (pin*2));           // Limpia bits
    GPIO->PUPDR |=  (pupd << (pin*2));        // Configura pull-up/pull-down

    GPIO->ODR &= ~(1 << pin);                // Limpia bit
    GPIO->ODR |=  (initial_value << pin);     // Configura valor inicial

    // No se configura la función alternativa en este proyecto
    // GPIOx_AFRL o GPIOx_AFRH según el pin
}

void gpio_setup_pin(GPIO_Typedef_t *GPIO, uint8_t pin, uint8_t mode /*2=AF*/, uint8_t af_num /*0..15*/)
{
    // Modo AF en MODER
    GPIO->MODER &= ~(3U << (pin*2));
    GPIO->MODER |=  ((mode & 3U) << (pin*2));

    // Selección de AF (AFRL para pines 0..7, AFRH para 8..15)
    if (pin < 8) {
        GPIO->AFRL &= ~(0xFU << (pin*4));
        GPIO->AFRL |=  ((af_num & 0xFU) << (pin*4));
    } else {
        uint8_t p = pin - 8;
        GPIO->AFRH &= ~(0xFU << (p*4));
        GPIO->AFRH |=  ((af_num & 0xFU) << (p*4));
    }
}

void set_gpio(GPIO_Typedef_t *GPIO, uint8_t pin){

        GPIO->ODR |= (1 << pin);  
}

void clear_gpio(GPIO_Typedef_t *GPIO, uint8_t pin){

        GPIO->ODR &= ~(1 << pin);  
}

uint8_t read_gpio(GPIO_Typedef_t *GPIO, uint8_t pin){
        // Leer estado del botón PC13 (botón presionado = 0, no presionado = 1)
    //if (!(GPIOC_IDR & (1 << B1_PIN))==0) {

    if ((GPIOC->IDR & (1 << B1_PIN))==0) {
        return 1;
    }
    return 0;
}
