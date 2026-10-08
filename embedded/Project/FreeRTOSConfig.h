#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* 基础配置 */
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ        ( 200000000UL ) // GD32F450主频
#define configTICK_RATE_HZ                      ( 1000 )
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( 128 )
#define configTOTAL_HEAP_SIZE                   ( 20480 )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define INCLUDE_vTaskDelay        1
#define INCLUDE_vTaskDelayUntil    1

/* 中断优先级配置 - 这是你缺少的部分！ */
#define configPRIO_BITS                         4                  /* GD32F4xx NVIC 使用 4 位优先级 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* 可选：断言和钩子函数 */
#define configASSERT( x )                       if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

#endif /* FREERTOS_CONFIG_H */
