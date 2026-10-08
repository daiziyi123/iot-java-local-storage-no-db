#include "app_project.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "trash_types.h"
#include <string.h> 

extern void vTaskTrashSensor(void *pvParameters);
extern void vTaskTrashControl(void *pvParameters);
extern void vTaskTrashWifi(void *pvParameters);
extern void vTaskTrashDisplay(void *pvParameters);

SemaphoreHandle_t xTrashDataMutex;
TrashData_t g_trashData;
QueueHandle_t xTrashCommandQueue;

void app_project_create_tasks_trash(void)
{
    xTrashDataMutex = xSemaphoreCreateMutex();
    xTrashCommandQueue = xQueueCreate(5, sizeof(char));
    if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
        memset(&g_trashData, 0, sizeof(g_trashData));
        xSemaphoreGive(xTrashDataMutex);
    }
    xTaskCreate(vTaskTrashSensor, "TrashSensor", 512, NULL, 4, NULL);
    xTaskCreate(vTaskTrashControl, "TrashCtrl",   512, NULL, 3, NULL);
    xTaskCreate(vTaskTrashWifi,   "TrashWifi",   1024, NULL,2 , NULL);
    xTaskCreate(vTaskTrashDisplay, "TrashDisp",   256, NULL, 1, NULL);
}