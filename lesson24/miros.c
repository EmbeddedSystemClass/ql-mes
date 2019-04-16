/****************************************************************************
* MInimal Real-time Operating System (MIROS)
* version 0.23 (matching lesson 23)
*
* This software is a teaching aid to illustrate the concepts underlying
* a Real-Time Operating System (RTOS). The main goal of the software is
* simplicity and clear presentation of the concepts, but without dealing
* with various corner cases, portability, or error handling. For these
* reasons, the software is generally NOT intended or recommended for use
* in commercial applications.
*
* Copyright (C) 2018 Miro Samek. All Rights Reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* Contact Information:
* https://www.state-machine.com
****************************************************************************/
#include <stdint.h>
#include "miros.h"
#include "qassert.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */


Q_DEFINE_THIS_FILE


OSThread * volatile OS_curr; /* pointer to the current thread */
OSThread * volatile OS_next; /* pointer to the next thread to run */

OSThread *OS_thread[32 + 1]; /* array of threads started so far */
uint8_t OS_threadNum; /* number of threads started so far */
uint8_t OS_currIdx; /* current thread index for round robin scheduling */

void OS_init(void) {
    /* set the PendSV interrupt priority to the lowest level 0xFF;
     * precisely: switch priorities of SysTick (from lowest to highest)
     * with PendSV from (highest to lowest)
     */
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);
}

 void OS_sched(void) {
     /* OS_next = ... */
     ++OS_currIdx;
     if (OS_currIdx == OS_threadNum) {
         OS_currIdx = 0U;
     }
     OS_next = OS_thread[OS_currIdx];

     /* trigger PendSV, if needed */
    if (OS_next != OS_curr) {
        *(uint32_t volatile *)0xE000ED04 = (1U << 28);
    }
}


 void OS_run(void) {
     /* callback to configure and start interrupts */
     OS_onStartup();

     __disable_irq();
     OS_sched();
     __enable_irq();

     /* the following code should never execute */
     Q_ERROR();
 }


void OSThread_start(
    OSThread *me,
    OSThreadHandler threadHandler,
    void *stkSto, uint32_t stkSize)
{
    /* round down the stack top to the 8-byte boundary
    * NOTE: ARM Cortex-M stack grows down from hi -> low memory
    */
    uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
    uint32_t *stk_limit;

    *(--sp) = (1U << 24);  /* xPSR */
    *(--sp) = (uint32_t)threadHandler; /* PC */
    *(--sp) = 0x0000000EU; /* LR  */
    *(--sp) = 0x0000000CU; /* R12 */
    *(--sp) = 0x00000003U; /* R3  */
    *(--sp) = 0x00000002U; /* R2  */
    *(--sp) = 0x00000001U; /* R1  */
    *(--sp) = 0x00000000U; /* R0  */
    /* additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU; /* R11 */
    *(--sp) = 0x0000000AU; /* R10 */
    *(--sp) = 0x00000009U; /* R9 */
    *(--sp) = 0x00000008U; /* R8 */
    *(--sp) = 0x00000007U; /* R7 */
    *(--sp) = 0x00000006U; /* R6 */
    *(--sp) = 0x00000005U; /* R5 */
    *(--sp) = 0x00000004U; /* R4 */

    /* save the top of the stack in the thread's attribute */
    me->sp = sp;

    /* round up the bottom of the stack to the 8-byte boundary */
    stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

    /* pre-fill the unused part of the stack with 0xDEADBEEF */
    for (sp = sp - 1U; sp >= stk_limit; --sp) {
        *sp = 0xDEADBEEFU;
    }

    Q_ASSERT(OS_threadNum < Q_DIM(OS_thread));

    /* register the thread with the OS */
    OS_thread[OS_threadNum] = me;
    ++OS_threadNum;
}


void PendSV_Handler(void) {

//    "IMPORT  OS_curr"  /* extern variable */
//    "IMPORT  OS_next"  /* extern variable */

    /* __disable_irq(); */
__asm("CPSID         I\n\t");

    /* if (OS_curr != (OSThread *)0) { */
__asm("LDR           r1,=OS_curr\n\t"
      "LDR           r1,[r1,#0x00]\n\t"
      "CBZ           r1,PendSV_restore\n\t");

    /*     push registers r4-r11 on the stack */
__asm("PUSH          {r4-r11}\n\t");

    /*     OS_curr->sp = sp; */
__asm("LDR           r1,=OS_curr\n\t"
      "LDR           r1,[r1,#0x00]\n\t"
      "STR           sp,[r1,#0x00]\n\t");
    /* } */

    /* sp = OS_next->sp; */
__asm("PendSV_restore:\n\t"
      "LDR           r1,=OS_next\n\t"
      "LDR           r1,[r1,#0x00]\n\t"
      "LDR           sp,[r1,#0x00]\n\t");

    /* OS_curr = OS_next; */
__asm("LDR           r1,=OS_next\n\t"
      "LDR           r1,[r1,#0x00]\n\t"
      "LDR           r2,=OS_curr\n\t"
      "STR           r1,[r2,#0x00]\n\t");

    /* pop registers r4-r11 */
__asm("POP           {r4-r11}\n\t");

    /* __enable_irq(); */
__asm("CPSIE         I\n\t");

    /* return to the next thread */
__asm("BX            lr\n\t");
}
