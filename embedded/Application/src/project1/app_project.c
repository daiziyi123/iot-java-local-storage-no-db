#include "app_project.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* 外部任务函数声明（在本文件夹的 .c 中实现） */
extern void vTaskSensorRead(void *pvParameters);
extern void vTaskControl(void *pvParameters);
extern void vTaskWifiAT(void *pvParameters);
extern void vTaskDisplayRefresh(void *pvParameters);

/* 项目0专用互斥锁和共享数据（定义在本文件，外部不可见） */
SemaphoreHandle_t xSensorMutex;
SensorData_t g_sensorData;
QueueHandle_t xCommandQueue;
void app_project_create_tasks(void)
{
    xSensorMutex = xSemaphoreCreateMutex();
    xCommandQueue = xQueueCreate(10, sizeof(Command_t));

    xTaskCreate(vTaskSensorRead, "Sensor", 256, NULL, 1, NULL);
    xTaskCreate(vTaskControl,    "Control",512, NULL, 3, NULL);
    xTaskCreate(vTaskWifiAT,     "Wifi",   1024, NULL, 4, NULL);
    xTaskCreate(vTaskDisplayRefresh,    "Display",256, NULL, 1, NULL);
}