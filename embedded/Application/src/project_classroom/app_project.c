#include "app_project.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "classroom_types.h"
#include "semphr.h"
#include "task.h"

extern void vTaskSensorRead(void *pvParameters);
extern void vTaskControl(void *pvParameters);
extern void vTaskWifiAT(void *pvParameters);
extern void vTaskDisplay(void *pvParameters);

SemaphoreHandle_t xClassroomMutex;
ClassroomData_t g_classroomData;
QueueHandle_t xClassroomCommandQueue;

void app_project_create_tasks_classroom(void)
{
    // 创建互斥量和队列
    xClassroomMutex = xSemaphoreCreateMutex();
    xClassroomCommandQueue = xQueueCreate(8, sizeof(char));

    if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
    {
        g_classroomData.temperature = 0.0f;
        g_classroomData.humidity = 0.0f;
        g_classroomData.light = 0.0f;
        g_classroomData.pir_detected = 0;
        g_classroomData.key_pressed = 0;
        g_classroomData.mode = 0; // 默认正常模式
        g_classroomData.fan_speed = 0;
        g_classroomData.led_brightness = 0;
        g_classroomData.curtain_pos = 0;
        g_classroomData.occupancy_sec = 0;
        g_classroomData.total_sec = 0;
        g_classroomData.fan_runtime = 0;
        g_classroomData.light_runtime = 0;
        g_classroomData.manual_lock = false;  
        xSemaphoreGive(xClassroomMutex);
    }
    // 创建任务
    xTaskCreate(vTaskSensorRead, "ClsSensor", 512, NULL, 4, NULL);
    xTaskCreate(vTaskControl, "ClsCtrl", 512, NULL, 3, NULL);
    xTaskCreate(vTaskWifiAT, "ClsWifi", 1024, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay, "ClsDisp", 256, NULL, 1, NULL);
}
