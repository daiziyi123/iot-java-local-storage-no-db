#include "app_project.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pet_types.h"          
#include <string.h>   
extern void vTaskSensorRead(void *pvParameters);
extern void vTaskControl(void *pvParameters);
extern void vTaskWifiAT(void *pvParameters);
extern void vTaskDisplay(void *pvParameters);
SemaphoreHandle_t xPetDataMutex;
PetData_t g_petData;
QueueHandle_t xPetCommandQueue;
void app_project_create_tasks_pet(void)
{
    xPetDataMutex = xSemaphoreCreateMutex();
    xPetCommandQueue = xQueueCreate(5, sizeof(char));
    if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
        memset(&g_petData, 0, sizeof(g_petData));
        xSemaphoreGive(xPetDataMutex);
    }
    xTaskCreate(vTaskSensorRead, "PetSensor", 512, NULL, 4, NULL);
    xTaskCreate(vTaskControl,    "PetCtrl",   512, NULL, 3, NULL);
    xTaskCreate(vTaskWifiAT,     "PetWifi",   1024, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay,    "PetDisp",   256, NULL, 1, NULL);
}