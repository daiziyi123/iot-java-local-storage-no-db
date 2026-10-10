#define configCPU_CLOCK_HZ        ( 200000000UL ) // GD32F470 200MHz PLL from 25MHz HXTAL
#define configTICK_RATE_HZ        ( 1000 )        // 1ms系统滴答
#define configMAX_PRIORITIES      ( 8 )
#define configMINIMAL_STACK_SIZE  ( 128 )
#define configTOTAL_HEAP_SIZE     ( ( size_t ) ( 16 * 1024 ) ) // 16KB堆，满足WiFi缓存
#define configMAX_TASK_NAME_LEN   ( 16 )
#define configUSE_PREEMPTION      1
#define configUSE_TIME_SLICING    1
#define configUSE_MUTEXES         1 // 开启互斥锁（代码依赖）
#define configUSE_IDLE_HOOK       0
#define configUSE_TICK_HOOK       0
#define configUSE_TRACE_FACILITY  0
#define configUSE_16_BIT_TICKS    0
#define configIDLE_SHOULD_YIELD   1
#define configUSE_APPLICATION_TASK_TAG 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configQUEUE_REGISTRY_SIZE 0
#define configENABLE_BACKWARD_COMPATIBILITY 0

/* FreeRTOS API functions to include */
#define INCLUDE_vTaskDelay             1
#define INCLUDE_vTaskDelayUntil        1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xSemaphoreGetMutexHolder 1

// SysTick中断适配FreeRTOS
#define xPortPendSVHandler PendSV_Handler
#define vPortSVCHandler SVC_Handler
#define xPortSysTickHandler SysTick_Handler