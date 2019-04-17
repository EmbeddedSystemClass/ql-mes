#include "bsp.h"

Q_DEFINE_THIS_FILE


uint32_t stack_blinky1[40];
QXThread blinky1;
void main_blinky1(QXThread * const me) {
    while (1) {
        uint32_t volatile i;
        for (i = 1500U; i != 0U; --i) {
            if (i%150 == 0) BSP_ledGreenOn();
            if (i%77 == 0) BSP_ledGreenOff();
        }
        QXThread_delay(1U); /* block for 1 tick */
    }
}

uint32_t stack_blinky2[40];
QXThread blinky2;
void main_blinky2(QXThread * const me) {
    while (1) {
        uint32_t volatile i;
        for (i = 3*1500U; i != 0U; --i) {
            if (i%200 == 0) BSP_ledBlueOn();
            if (i%33 == 0) BSP_ledBlueOff();
        }
        QXThread_delay(10U); /* block for 10 ticks */
    }
}

uint32_t stack_blinky3[40];
QXThread blinky3;
void main_blinky3(QXThread * const me) {
    while (1) {
        BSP_ledRedOn();
        QXThread_delay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        QXThread_delay(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}


/* background code: sequential with blocking version */
int main() {
    QF_init();
    BSP_init();

	QXThread_ctor(&blinky1, &main_blinky1, 0);
    /* start blinky1 thread */
    QXTHREAD_START(&blinky1,
                   5U,
                   (void *)0, 0, /* message queue buffer and size */
                   stack_blinky1, sizeof(stack_blinky1),
                   (void *)0);

    QXThread_ctor(&blinky2, &main_blinky2, 0);
    /* start blinky2 thread */
    QXTHREAD_START(&blinky2,
                   2U,
                   (void *)0, 0, /* message queue buffer and size */
                   stack_blinky2, sizeof(stack_blinky2),
                   (void *)0);

    QXThread_ctor(&blinky3, &main_blinky3, 0);
    /* start blinky3 thread */
    QXTHREAD_START(&blinky3,
                   1U,
                   (void *)0, 0, /* message queue buffer and size */
                   stack_blinky3, sizeof(stack_blinky3),
                   (void *)0);



    QF_run();

    //return 0;
}
