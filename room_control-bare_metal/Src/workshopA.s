// --- Botón B1 (PC13) enciende LD2 (PA5) por 3 s con SysTick=1 ms -----------
//     STM32L476xG / Nucleo-L476RG
    .section .text
    .syntax unified
    .thumb

    .global main
    .global init_led
    .global init_button
    .global init_systick
    .global SysTick_Handler

// --- Definiciones RCC/GPIO ---------------------------------------------------
    .equ RCC_BASE,        0x40021000           @ Base de RCC
    .equ RCC_AHB2ENR,     RCC_BASE + 0x4C      @ Enable GPIOA clock (AHB2ENR)

    .equ GPIOA_BASE,      0x48000000           @ Base de GPIOA
    .equ GPIOA_MODER,     GPIOA_BASE + 0x00    @ Mode register
    .equ GPIOA_ODR,       GPIOA_BASE + 0x14    @ Output data register

    .equ GPIOC_BASE,      0x48000800           @ Base de GPIOC
    .equ GPIOC_MODER,     GPIOC_BASE + 0x00    @ Mode register
    .equ GPIOC_IDR,       GPIOC_BASE + 0x10    @ Input data register

    .equ LD2_PIN,         5                    @ Pin del LED LD2
    .equ BUTTON_PIN,      13                   @ PC13 (B1)

// --- SysTick -----------------------------------------------------------------
    .equ SYST_CSR,        0xE000E010        @ Control & Status
    .equ SYST_RVR,        0xE000E014        @ Reload
    .equ SYST_CVR,        0xE000E018        @ Current
    .equ HSI_FREQ,        4000000           @ 4 MHz (MSI/HSI por defecto)
    .equ SYSTICK_RELOAD,  3999              @ 1 ms @ 4 MHz: 4000-1 / Valor de recarga para generar 1 ms
    .equ TICKS_3S,        3000              @ 3000 ms / Número de ticks (1 ms cada uno) para 3 s

// --- BSS: variables de estado ------------------------------------------------
    .section .bss
    .align 4                                @ Contador de tiempo restante para mantener el LED encendido
led_ticks:
    .word 0                                 @ Cuenta regresiva en ms
btn_prev:                                   @ Memoria del último estado del botón para detectar cambios.
    .word 0                                 @ Estado previo del botón (0/1)

    .section .text

// --- Programa principal ------------------------------------------------------
main:
    bl  init_led
    bl  init_button
    bl  init_systick

    // Inicializa btn_prev con el estado actual de PC13
    movw r0, #:lower16:GPIOC_IDR
    movt r0, #:upper16:GPIOC_IDR
    ldr  r1, [r0]
    lsrs r1, r1, #BUTTON_PIN
    ands r1, r1, #1
    ldr  r2, =btn_prev      //Guarda el estado previo
    str  r1, [r2]

loop:
    // Lee PC13
    movw r0, #:lower16:GPIOC_IDR
    movt r0, #:upper16:GPIOC_IDR
    ldr  r1, [r0]
    lsrs r1, r1, #BUTTON_PIN
    ands r1, r1, #1            @ r1 = estado actual (0/1)

    // Carga estado previo
    ldr  r2, =btn_prev
    ldr  r3, [r2]              @ r3 = previo

    // Detecta flanco ascendente: previo=0 y actual=1
    cmp  r3, #0
    bne  no_press
    cmp  r1, #1
    bne  no_press

pressed:
    // Enciende LED (PA5 = 1)
    movw r0, #:lower16:GPIOA_ODR
    movt r0, #:upper16:GPIOA_ODR
    ldr  r4, [r0]
    orr  r4, r4, #(1 << LD2_PIN)
    str  r4, [r0]

    // Carga temporizador a 3000 ms
    ldr  r0, =led_ticks
    ldr  r4, =TICKS_3S
    str  r4, [r0]

no_press:
    // Actualiza previo = actual
    ldr  r2, =btn_prev
    str  r1, [r2]

    b    loop

// --- Inicialización de GPIOA PA5 (LD2) --------------------------------------
init_led:
    // Habilita reloj GPIOA (bit0)
    movw r0, #:lower16:RCC_AHB2ENR
    movt r0, #:upper16:RCC_AHB2ENR
    ldr  r1, [r0]
    orr  r1, r1, #(1 << 0)
    str  r1, [r0]

    // PA5 como salida (MODER5 = 01)
    movw r0, #:lower16:GPIOA_MODER
    movt r0, #:upper16:GPIOA_MODER
    ldr  r1, [r0]
    bic  r1, r1, #(0b11 << (LD2_PIN * 2))
    orr  r1, r1, #(0b01 << (LD2_PIN * 2))
    str  r1, [r0]
    bx   lr

// --- Inicialización de GPIOC PC13 (B1) como entrada -------------------------
init_button:
    // Habilita reloj GPIOC (bit2)
    movw r0, #:lower16:RCC_AHB2ENR
    movt r0, #:upper16:RCC_AHB2ENR
    ldr  r1, [r0]
    orr  r1, r1, #(1 << 2)
    str  r1, [r0]

    // PC13 como entrada (MODER13 = 00)
    movw r0, #:lower16:GPIOC_MODER
    movt r0, #:upper16:GPIOC_MODER
    ldr  r1, [r0]
    bic  r1, r1, #(0b11 << (BUTTON_PIN * 2))
    str  r1, [r0]
    bx   lr

// --- Inicialización de SysTick a 1 ms ---------------------------------------
init_systick:
    // Reload = 3999 (1 ms @ 4 MHz)
    movw r0, #:lower16:SYST_RVR
    movt r0, #:upper16:SYST_RVR
    ldr  r1, =SYSTICK_RELOAD
    str  r1, [r0]

    // Limpia contador actual
    movw r0, #:lower16:SYST_CVR
    movt r0, #:upper16:SYST_CVR
    movs r1, #0
    str  r1, [r0]

    // ENABLE=1, TICKINT=1, CLKSOURCE=1
    movw r0, #:lower16:SYST_CSR
    movt r0, #:upper16:SYST_CSR
    movs r1, #(1<<0)|(1<<1)|(1<<2)
    str  r1, [r0]
    bx   lr

// --- Manejador SysTick: decrementa temporizador y apaga LED al llegar a 0 ---
    .thumb_func
SysTick_Handler:
    // led_ticks > 0 ? led_ticks-- : nada
    ldr  r0, =led_ticks
    ldr  r1, [r0]
    cbz  r1, syst_done          @ si 0, salir
    subs r1, r1, #1
    str  r1, [r0]
    bne  syst_done              @ si aún >0, salir

    // Llegó a 0: apaga LED (PA5 = 0)
    movw r2, #:lower16:GPIOA_ODR
    movt r2, #:upper16:GPIOA_ODR
    ldr  r3, [r2]
    bic  r3, r3, #(1 << LD2_PIN)
    str  r3, [r2]

syst_done:
    bx   lr

