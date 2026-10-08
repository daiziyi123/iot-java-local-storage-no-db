#include "app_project.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "drone_types.h"

/* 全局共享数据与同步对象 */
SemaphoreHandle_t xDroneDataMutex;
DroneData_t g_droneData;
QueueHandle_t xDroneCommandQueue;

/* 外部任务声明 */
extern void vTaskSensorRead(void *pvParameters);
extern void vTaskControl(void *pvParameters);
extern void vTaskWifiAT(void *pvParameters);
extern void vTaskDisplay(void *pvParameters);

void app_project_create_tasks_drone(void)
{
    xDroneDataMutex = xSemaphoreCreateMutex();
    xDroneCommandQueue = xQueueCreate(5, sizeof(char));
    if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
            g_droneData.altitude    = 0.0f;      // 初始高度0
            g_droneData.temperature = 25.0f;     // 默认室温25°C
            g_droneData.humidity    = 60.0f;     // 默认湿度60%
            g_droneData.power_gear  = 0;         // 低档位
            g_droneData.alarm_flag  = 0;         // 无报警
            g_droneData.flight_mode = 0;         // 待命模式
            g_droneData.flight_time = 0;         // 飞行时间清零
            xSemaphoreGive(xDroneDataMutex);
    }
    xTaskCreate(vTaskSensorRead, "DroneSensor", 512, NULL, 4, NULL);
    xTaskCreate(vTaskControl,    "DroneCtrl",   512, NULL, 3, NULL);
    xTaskCreate(vTaskWifiAT,     "DroneWifi",   1024, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay,    "DroneDisp",   256, NULL, 1, NULL);
}