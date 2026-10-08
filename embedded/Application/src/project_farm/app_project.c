#include "app_project.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "farm_types.h"
#include <string.h> 
extern void vTaskFarmSensor(void *pvParameters);
extern void vTaskFarmControl(void *pvParameters);
extern void vTaskFarmWifi(void *pvParameters);
extern void vTaskFarmDisplay(void *pvParameters);

SemaphoreHandle_t xFarmDataMutex;
FarmData_t g_farmData;
QueueHandle_t xFarmCommandQueue;

void app_project_create_tasks_farm(void)
{
    xFarmDataMutex = xSemaphoreCreateMutex();
    xFarmCommandQueue = xQueueCreate(5, sizeof(char));

    if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
        memset(&g_farmData, 0, sizeof(g_farmData));
        xSemaphoreGive(xFarmDataMutex);
    }
    xTaskCreate(vTaskFarmSensor, "FarmSensor", 512, NULL, 4, NULL);
    xTaskCreate(vTaskFarmControl, "FarmCtrl",   512, NULL, 3, NULL);
    xTaskCreate(vTaskFarmWifi,   "FarmWifi",   1024, NULL, 2, NULL);
    xTaskCreate(vTaskFarmDisplay, "FarmDisp",   256, NULL, 1, NULL);
}