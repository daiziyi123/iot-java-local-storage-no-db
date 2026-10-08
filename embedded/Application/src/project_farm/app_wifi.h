#ifndef APP_WIFI_H
#define APP_WIFI_H

void vTaskFarmWifi(void *pvParameters);
void send_sensor_data_to_server(void);
#endif