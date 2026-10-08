#include "app_project.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "window_types.h"

extern void vTaskSensorRead(void *pvParameters);
extern void vTaskControl(void *pvParameters);
extern void vTaskWifiAT(void *pvParameters);
extern void vTaskDisplay(void *pvParameters);

SemaphoreHandle_t xWindowMutex;
windowData_t g_windowData;
QueueHandle_t xWindowCommandQueue;

void app_project_create_tasks_window(void)
{
    // 创建互斥量和队列
    xWindowMutex = xSemaphoreCreateMutex();
    xWindowCommandQueue = xQueueCreate(8, sizeof(char));
    if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
    g_windowData.temperature  = 25.0f;    // 默认室温25°C
    g_windowData.humidity     = 60.0f;    // 默认湿度60%
    g_windowData.light        = 300.0f;   // 默认室内常规照度300lux
    g_windowData.pir_detected = 0;         // 初始未检测到人体
    
    g_windowData.led_brightness = 50;      // LED初始亮度50%
    g_windowData.curtain_pos    = 0;       // 初始窗户全关
    g_windowData.opening        = 0;       // 初始窗户处于静止状态
    g_windowData.ifauto         = 1;       // 初始默认手动模式
    
    g_windowData.occupancy_sec    = 0;     // 累计有人时长清零
    g_windowData.total_sec        = 0;     // 系统运行总时长清零
    g_windowData.curtain_runtime  = 0;     // 窗户运行时长清零
    xSemaphoreGive(xWindowMutex);
}

    // 创建任务
    xTaskCreate(vTaskSensorRead, "ClsSensor", 1024, NULL, 4, NULL);
    xTaskCreate(vTaskControl,    "ClsCtrl",   512, NULL, 3, NULL);
    xTaskCreate(vTaskWifiAT,     "ClsWifi",   1024, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay,    "ClsDisp",   256, NULL, 1, NULL);
}