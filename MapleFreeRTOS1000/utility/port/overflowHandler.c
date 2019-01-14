
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                   signed char *pcTaskName) {
    /* This function will get called if a task overflows its stack.
     * If the parameters are corrupt then inspect pxCurrentTCB to find
     * which was the offending task. */

    (void) pxTask;
    (void) pcTaskName;

    while (1)
    {

    }
        ;
}

