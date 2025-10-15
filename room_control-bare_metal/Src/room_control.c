// room_control.c
#include "room_control.h"
#include "systick.h"
#include "uart.h"
#include "tim.h"
#include <complex.h>

// Usamos el contador global de 1 ms
extern volatile uint32_t ms_counter;
static inline uint32_t systick_get_ms(void){ return ms_counter; }

// Estados básicos
typedef enum {
    ROOM_IDLE = 0,
    ROOM_OCCUPIED
} room_state_t;

static room_state_t current_state = ROOM_IDLE;
static uint32_t last_action_time = 0;

// Parser para comando "B<0-9>"
static uint8_t awaiting_b_level = 0;

// Aplica salidas según estado (solo PWM)
static void apply_state_outputs(void){
    if (current_state == ROOM_OCCUPIED) {
        tim3_ch1_pwm_set_duty_cycle(100); // 100% en ocupado
    } else {
        tim3_ch1_pwm_set_duty_cycle(0);   // 0% en idle
    }
}

void room_control_app_init(void)
{
    current_state    = ROOM_IDLE;
    last_action_time = systick_get_ms();
    awaiting_b_level = 0;
    apply_state_outputs();
    uart_send_string("Room Control listo\r\n");
}

void room_control_on_button_press(void)
{
    if (current_state == ROOM_IDLE) {
        current_state    = ROOM_OCCUPIED;
        last_action_time = systick_get_ms();
        apply_state_outputs();
        uart_send_string("Sala ocupada\r\n");
    } else {
        current_state = ROOM_IDLE;
        apply_state_outputs();
        uart_send_string("Sala vacia\r\n");
    }
}

void room_control_on_uart_receive(char received_char)
{
    // Ignorar CR/LF para que 'B' + Enter no rompa el parser
    if (received_char == '\r' || received_char == '\n') {
        if (awaiting_b_level) return;
        return;
    }

    // ¿Estábamos esperando el dígito tras 'B'?
    if (awaiting_b_level) {
        if (received_char >= '0' && received_char <= '9') {
            uint8_t level = (uint8_t)(received_char - '0'); // 0..9
            tim3_ch1_pwm_set_duty_cycle((uint8_t)(level * 10U));
            uart_send_string("PWM actualizado\r\n");
            awaiting_b_level = 0;
            return;
        }
        if (received_char == 'B' || received_char == 'b') {
            awaiting_b_level = 1;
            uart_send_string("Ingrese nivel 0-9\r\n");
            return;
        }
        awaiting_b_level = 0;
        uart_send_string("Nivel invalido (use B0..B9)\r\n");
        return;
    }

    // Comandos directos
    switch (received_char) {
        case 'O': case 'o':   // Forzar IDLE
            current_state = ROOM_IDLE;
            apply_state_outputs();
            uart_send_string("Sala vacia\r\n");
            break;

        case 'I': case 'i':   // Forzar OCCUPIED (reinicia timeout)
            current_state    = ROOM_OCCUPIED;
            last_action_time = systick_get_ms();
            apply_state_outputs();
            uart_send_string("Sala ocupada\r\n");
            break;

        case 'B': case 'b':   // Ajuste manual de duty: B0..B9
            awaiting_b_level = 1;
            uart_send_string("Ingrese nivel 0-9\r\n");
            break;

        default:              // Eco
            uart_send(received_char);
            break;
    }
}

void room_control_update(void)
{
    if (current_state == ROOM_OCCUPIED) {
        uint32_t now = systick_get_ms();
        if ((uint32_t)(now - last_action_time) >= LED_TIMEOUT_MS) {
            current_state = ROOM_IDLE;
            apply_state_outputs();
            uart_send_string("Timeout -> Sala vacia\r\n");
        }
    }
}
