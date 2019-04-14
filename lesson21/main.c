#include <stdint.h>
#include "bsp.h"
#include "TM4C123GH6PM.h"



int main() {
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    SysTick->LOAD = SYS_CLOCK_HZ/2U - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 2) | (1U << 1) | 1U;

    SysTick_Handler();

    __enable_irq();
    while (1) {
        GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
        BSP_delay(BSP_TICKS_PER_SEC / 4U);
        GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
        BSP_delay(BSP_TICKS_PER_SEC / 4U);
    }
    //return 0;
}
