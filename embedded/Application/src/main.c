#if 1
#include "gd32f4xx.h"
#include "delay.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "app_common.h"
#include "app_hardware.h"
#include <stdio.h>
/* 外设驱动头文件（用于初始化） */
#include "e1.h"
#include "c3.h"
#include "s8.h"
#include "s2.h"
#include "e3.h"
#include "e2.h"
#include "s1.h"
#include "s6.h"
#include "s7.h"
#include "i2c.h"
#define PROJECT_ID 20

#if PROJECT_ID == 1
    #include "Project1/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks
#elif PROJECT_ID == 13
    #include "Project_Pet/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_pet
#elif PROJECT_ID == 20
    #include "Project_classroom/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_classroom
#elif PROJECT_ID == 15
    #include "Project_window/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_window
#elif PROJECT_ID == 25
    #include "Project_drone/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_drone
#elif PROJECT_ID == 16
    #include "Project_trash/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_trash
#elif PROJECT_ID == 17
    #include "Project_farm/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_farm
#else
    #error "Unknown PROJECT_ID"
#endif

/* 全局互斥锁（所有项目共享） */
SemaphoreHandle_t xUartMutex;
SemaphoreHandle_t xI2CMutex;

int i2c_reg_bytes_read(i2c_slave_info info, unsigned char reg, unsigned char * pbytes, unsigned char count);
static void BSP_Init(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    timer3_init();
    uart_init(USART0);
    uart_init(USART5);
    uart_init(USART1);
/* 初始化所有外设（未使用的项目不会调用，但初始化无害） */
    i2c_init();
    e1_tube_info = e1_tube_init();
    e1_led_info  = e1_led_init();
    e2_fan_info  = e2_fan_init();
    e3_curtain_info = e3_curtain_init();
    s8_ths_info  = s8_ths_init();
    s2_illuminance_info = s2_illuminance_init();
    s1_key_info  = s1_key_init();
    s6_ultrasonic_info = s6_ultrasonic_init();
    s7_ir_info  = s7_ir_init();
}

/* ---------- printf 重定向（带 UART 互斥） ---------- */
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
FILE __stdout;
void _sys_exit(int x) { x = x; }
void _ttywrch(int ch) { ch = ch; }
int fputc(int ch, FILE *f)
{
    // 不再在此处加锁，假设调用者已经持有 xUartMutex
    usart_data_transmit(USART1, ch);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
    return ch;
}

static void AppTaskCreate(void)
{
    xUartMutex = xSemaphoreCreateMutex();
    xI2CMutex  = xSemaphoreCreateMutex();
    APP_CREATE_TASKS();  // 调用当前项目的创建函数
}

int main(void)
{
    BSP_Init();
    AppTaskCreate();
    vTaskStartScheduler();
    while (1);
}



#endif
/* 实现数码管显示(U1、E1) */
#if 0
#include <stdio.h>
#include <string.h>
#include "gd32f4xx.h"
#include "delay.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "c3.h"          // ESP32-C3 AT 驱动

/* ====================== 全局资源 ====================== */
SemaphoreHandle_t xUartMutex;   // 保护 printf

// WiFi 状态变量
static uint8_t c3_result;
static int tcp_conn_established = 0;
static int wifi_run_state = 1;
static char recv_buffer[256];

/* ====================== printf 重定向（带互斥锁） ====================== */
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
FILE __stdout;
void _sys_exit(int x) { x = x; }
void _ttywrch(int ch) { ch = ch; }
int fputc(int ch, FILE *f)
{
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) {
        usart_data_transmit(USART1, ch);
        while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
        xSemaphoreGive(xUartMutex);
    }
    return ch;
}

/* ====================== WiFi 测试任务 ====================== */
static void vTaskWiFiTest(void *pvParameters)
{
    // 1. 初始化 ESP32-C3 AT 模块
    printf("\r\n=== WiFi TCP Test Start ===\r\n");
    c3_result = c3_init();
    if (!c3_result) {
        printf("ESP32-C3 AT Init FAILED!\r\n");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }  // 停止
    } else {
        printf("ESP32-C3 AT Init OK\r\n");
    }

    TickType_t lastHeartbeat = xTaskGetTickCount();
    int heartbeat_cnt = 0;

    for (;;) {
        // ---------- 检查 TCP 连接 ----------
        if (tcp_conn_established == 0) {
            printf("[WiFi] TCP not connected, trying to establish...\r\n");
            c3_wifi_tcp_lead(&wifi_run_state, &tcp_conn_established);
            if (tcp_conn_established) {
                printf("[WiFi] TCP connected successfully!\r\n");
            } else {
                printf("[WiFi] TCP connection failed, retry later.\r\n");
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        // ---------- TCP 已连接：接收数据 ----------
        int recv_len = c3_wifi_tcp_receive(recv_buffer, 1000);  // 阻塞等待 1000ms
        if (recv_len > 0) {
            recv_buffer[recv_len] = '\0';
            printf("[Rx] %s\r\n", recv_buffer);
        } else {
            // 超时（无数据），发送心跳包检测连接是否存活
            if ((xTaskGetTickCount() - lastHeartbeat) >= pdMS_TO_TICKS(10000)) {
                lastHeartbeat = xTaskGetTickCount();
                // 发送一个心跳消息，例如 "PING"
                char ping[] = "PING\r\n";
                int ret = c3_wifi_tcp_send(ping);
                if (ret < 0) {
                    printf("[WiFi] Send failed, connection may be broken, will reconnect.\r\n");
                    tcp_conn_established = 0;  // 触发重连
                } else {
                    printf("[WiFi] Heartbeat sent (PING)\r\n");
                }
            }
        }

        // 每 500ms 循环一次，处理重连检测
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ====================== 硬件初始化 ====================== */
static void BSP_Init(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    timer3_init();

    uart_init(USART0);
    uart_init(USART5);
    uart_init(USART1);
}

/* ====================== 主函数 ====================== */
int main(void)
{
    BSP_Init();

    // 创建互斥锁（用于 printf）
    xUartMutex = xSemaphoreCreateMutex();

    // 创建唯一的 WiFi 测试任务（优先级设为 2，适中）
    xTaskCreate(vTaskWiFiTest, "WiFiTest", 1024, NULL, 2, NULL);

    // 启动调度器
    vTaskStartScheduler();

    while (1);  // 永不执行
}
#endif

/* 实现风扇转速控制及显示(U1、E1、E2) */
#if 0
#include "gd32f4xx.h"
//#include "stdio.h"
#include "string.h"
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "e2.h"
#include "uart.h"
//printf函数重定向到 USART1 串口                                
#pragma import(__use_no_semihosting)

// 标准库需要的支持函数
struct __FILE {
    int handle;
};
FILE __stdout;

void _sys_exit(int x) {
    x = x;
    while(1);
}

void _ttywrch(int ch) {
    ch = ch;
}

// 重定向 fputc 到串口（假设已初始化 USART1）
int fputc(int ch, FILE *f) {
    // 发送一个字符，等待发送完成
    usart_data_transmit(USART1, (uint8_t)ch);
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
    return ch;
}
int main()
{
uart_init(USART1);   // 如果你有串口初始化函数
printf("Program started\n");
	char buf[10] = {0};
  
	e1_tube_info = e1_tube_init();
    
    printf("Tube init failed!\n");

e2_fan_info = e2_fan_init();

    printf("Fan init failed!\n");

	while(1)
	{	
		/* 循环递增i的值 */
		for(unsigned char i=0; i<=100; i+=10)
		{
			/* 将i的值转换为字符串并写入buf */
			sprintf(buf, "%d", i);
			/* 在数码管上显示buf中的字符串 */
			e1_tube_str_set(e1_tube_info, buf);
			/* 将风扇转速设置为i% */
			e2_fan_speed_set(e2_fan_info, i);
			printf("i=%d\n", i);  
			
		}		
	}
}
#endif

/*实现风扇匀速30转动效果*/
#if 0
#include <stdio.h>
#include "e2.h"
int main()
{
  /* 风扇初始化 */
	e2_fan_info = e2_fan_init();
	/* 将风扇转速设置为i% */
	e2_fan_speed_set(e2_fan_info, 30);
}
#endif


/* 实现窗帘位置控制及显示(U1、E1、E3) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "e3.h"

int main()
{
	char buf[10] = {0};
	/* 窗帘目标位置 */
	unsigned char target_position = 0;
	/* 窗帘当前位置 */
	unsigned char real_position;
	/* 窗帘电机状态，0表示电机停止工作，1表示电机正在工作 */
	unsigned char real_status;

	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* 窗帘初始化 */
	e3_curtain_info = e3_curtain_init();
	/* 将窗帘目标位置设置为target_position */
	e3_curtain_position_set(e3_curtain_info, target_position);
	
	while(1)
	{	
		/* 读取窗帘电机工作状态 */
		real_status = e3_curtain_status_get(e3_curtain_info);		
		/* 读取窗帘当前实际位置 */
		real_position = e3_curtain_position_get(e3_curtain_info);
		/* 将窗帘电机工作状态和窗帘当前实际位置转换为字符串并写入buf */
		sprintf(buf, "%d.%03d", real_status, real_position);
		/* 在数码管上显示buf中的字符串 */
		e1_tube_str_set(e1_tube_info, buf);
		/* 如果窗帘当前实际位置已经达到设置的目标位置且电机已经停止工作 */
		if(real_position == target_position && real_status == 0)
		{
			/* 延时1000ms */
			delay_ms(1000);
			/* 如果当前窗帘目标位置为0，则将窗帘目标位置修改为100 */
			if(target_position == 0)
			{
				target_position = 100;
			}
			/* 如果当前窗帘目标位置为100，则将窗帘目标位置修改为0 */
			else if(target_position == 100)
			{
				target_position = 0;
			}
			/* 将窗帘位置设置为target_position */
			e3_curtain_position_set(e3_curtain_info, target_position);
		}		
	}
}
#endif

/*编写代码让窗帘机滑动打开*/
#if 0
#include <stdio.h>
#include "e3.h"
int main()
{
 /* 窗帘初始化 */
 e3_curtain_info = e3_curtain_init();
	/*将滑块位置设置到目标点为百分之0的位置上*/
 e3_curtain_position_set(e3_curtain_info,0);
}
#endif

/* 实现按键检测及显示(U1、E1、S1) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "s1.h"

int main()
{
	char key = 0;

	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* 按键初始化 */
	s1_key_info = s1_key_init();

	while(1)
	{	
		/* 读取按键键值 */
		key = s1_key_value_get(s1_key_info);
		/* 如果读取到的键值是'0'~'9'，在数码管上显示对应的值 */
		if(key >= '0' && key <= '9')
		{
			e1_tube_str_set(e1_tube_info, &key);
		}
		/* 如果读取到的键值是'*'，在数码管上显示'd' */
		else if(key == '*')
		{
			e1_tube_str_set(e1_tube_info, "d");
		}
		/* 如果读取到的键值是'#'，在数码管上显示'o' */
		else if(key == '#')
		{
			e1_tube_str_set(e1_tube_info, "o");
		}
		else
		{
			
		}
		/* 延时100ms */
		delay_ms(100);
	}
}
#endif

/* 实现光照强度测量及显示(U1、E1、S2) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "s2.h"

int main()
{
	char buf[10] = {0};
	float illuminance = 0.0f;

	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* 光照强度传感器初始化 */
	s2_illuminance_info = s2_illuminance_init();
	while(1)
	{	
		/* 读取光照强度传感器测量结果（勒克斯） */
		illuminance = s2_illuminance_value_get(s2_illuminance_info);
		/* 将光照强度测量结果转换为字符串并写入buf */
		sprintf(buf, "%.0f", illuminance);
		/* 在数码管上显示buf中的字符串 */
		e1_tube_str_set(e1_tube_info, buf);
		/* 延时100ms */
		delay_ms(100);
	}
}
#endif

/* 实现加速度（z轴）测量及显示(U1、E1、S2) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "s2.h"

int main()
{
	char buf[10] = {0};
	s2_imu_t imu_value;

	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* 加速度&角速度传感器初始化 */
	s2_imu_info = s2_imu_init();

	while(1)
	{	
		/* 读取加速度&角速度传感器测量结果（m/s2、rad/s） */
		imu_value = s2_imu_value_get(s2_imu_info);
		/* 将z轴的加速度值转换为字符串并写入buf */
		sprintf(buf, "%.1f", imu_value.acc_z);
		/* 在数码管上显示buf中的字符串 */
		e1_tube_str_set(e1_tube_info, buf);
		/* 延时100ms */
		delay_ms(100);
	}
}
#endif

/* 实现角速度（z轴）测量及显示(U1、E1、S2) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "s2.h"

int main()
{
	char buf[10] = {0};
	s2_imu_t imu_value;

	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* 加速度&角速度传感器初始化 */
	s2_imu_info = s2_imu_init();

	while(1)
	{	
		/* 读取加速度&角速度传感器测量结果（m/s2、rad/s） */
		imu_value = s2_imu_value_get(s2_imu_info);
		/* 将z轴的角速度值转换为字符串并写入buf */
		sprintf(buf, "%.1f", imu_value.gyr_z);
		/* 在数码管上显示buf中的字符串 */
		e1_tube_str_set(e1_tube_info, buf);
		/* 延时100ms */
		delay_ms(100);
	}
}
#endif

/* 实现NFC的寻卡、防冲撞、选卡、认证、写卡、读卡、休眠等操作并显示相关信息(U1、E1、S5) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "s5.h"

int main()
{
	//状态机状态
	char Status = 0;
	//卡类型
	unsigned char CardType[2] = {0};
	//卡ID（全球唯一）
	unsigned char CardID[4] = {0};
	//卡密码
	unsigned char Key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	//数据缓冲
	char Data[16] = {0};

	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* NFC初始化 */
	s5_nfc_info = s5_nfc_init();
	
	while(1)
	{	
		//查询状态机当前状态
		switch(Status)
		{
			//寻卡状态
			case 0:
				//寻找射频场内所有符合ISO 14443-A协议且未休眠的卡(0x26)，寻卡成功后将卡的类型写入CardType
				if( s5_nfc_request(s5_nfc_info, 0x26, CardType) == MI_OK )
				{
					//如果寻卡成功，显示卡的类型
					sprintf(Data, "%02x%02x", CardType[0], CardType[1]);
					e1_tube_str_set(e1_tube_info, Data);
					delay_ms(1000);

					//如果寻卡成功，进入防冲撞状态
					Status = 1;
				}
				else
				{
					//如果寻卡失败，显示"----"
					sprintf(Data, "----");
					e1_tube_str_set(e1_tube_info, Data);
					delay_ms(1000);

					//如果寻卡失败，保持寻卡状态
					Status = 0;
				}
				break;
				
			//防冲撞状态
			case 1:
				//通过防冲撞机制选择射频场内的一张卡，防冲撞成功后将选中的卡的ID写入CardID
				if( s5_nfc_anticoll(s5_nfc_info, CardID) == MI_OK )
				{
					//如果防冲撞成功，显示卡的ID
					sprintf(Data, "%02x%02x%02x%02x", CardID[0], CardID[1], CardID[2], CardID[3]);
					e1_tube_str_set(e1_tube_info, Data);
					delay_ms(1000);
					//如果防冲撞成功，进入选卡状态
					Status = 2;					
				}
				else
				{
					//如果防冲撞失败，回到寻卡状态
					Status = 0;					
				}
				break;

			//选卡状态
			case 2:
				//选中ID为CardID的卡
				if( s5_nfc_select(s5_nfc_info, CardID) == MI_OK )
				{
					//如果选卡成功，进入认证/休眠状态
					Status = 3;
					//Status = 6;	
				}
				else
				{
					//如果选卡失败，回到寻卡状态
					Status = 0;					
				}
				break;
				
			//认证状态
			case 3:
				//使用秘钥A(0x60)认证ID为CardID的卡的第5块（第1区的第1块），认证密码为Key
				if( s5_nfc_auth(s5_nfc_info, 0x60, 5, Key, CardID) == MI_OK )
				{
					//如果认证成功，进入写卡/读卡状态
					Status = 4;
					//Status = 5;						
				}
				else
				{
					//如果认证失败，回到寻卡状态
					Status = 0;					
				}
				break;
			
			//写卡状态
			case 4:
				//向第5块（第1区的第1块）中写入数据(16Byte)"1234567887654321"
				if( s5_nfc_write(s5_nfc_info, 5, (unsigned char *)"1234567887654321") == MI_OK )
				{	
					//如果写卡成功，进入读卡/休眠状态
					Status = 5;		
					//Status = 6;					
				}
				else
				{
					//如果写卡失败，回到寻卡状态
					Status = 0;					
				}
				break;
				
			//读卡状态
			case 5:
				//读取第5块（第1区的第1块）中的数据(16Byte)，读取成功后将数据写入Data
				if( s5_nfc_read(s5_nfc_info, 5, (unsigned char *)Data) == MI_OK )
				{		
					//如果读卡成功，显示读取到的数据
					e1_tube_str_set(e1_tube_info, Data);
					delay_ms(1000);
					
					//如果读卡成功，进入写卡/休眠状态
					//Status = 4;					
					Status = 6;
				}
				else
				{
					//如果读卡失败，回到寻卡状态
					Status = 0;					
				}
				break;
				
			//休眠状态
			case 6:
				//休眠当前选中的卡
				if( s5_nfc_halt(s5_nfc_info) == MI_OK )
				{
					//如果休眠成功，回到寻卡状态
					Status = 0;					
				}
				else
				{
					//如果休眠失败，回到寻卡状态
					Status = 0;					
				}
				break;
				
			default:
				break;
		}
	}
}
#endif




/* 实现超声波测距及显示(U1、E1、S6) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "s6.h"

int main()
{
	char buf[10] = {0};
	unsigned int distance = 0;

	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* 超声波传感器初始化 */
	s6_ultrasonic_info = s6_ultrasonic_init();

	while(1)
	{	
		/* 读取超声波传感器测量结果（毫米） */
		distance = s6_ultrasonic_distance_get(s6_ultrasonic_info);
		/* 将测量结果转换为字符串并写入buf */
		sprintf(buf, "%d", distance);
		/* 在数码管上显示buf中的字符串 */
		e1_tube_str_set(e1_tube_info, buf);
		/* 延时100ms */
		delay_ms(100);
	}
}
#endif

		/* 实现人体感应及显示(U1、E1、S7) */
		#if 0
		#include <stdio.h>
		#include "delay.h"
		#include "e1.h"
		#include "s7.h"
		int main()
		{
			unsigned char status = 0;
			/* 数码管初始化 */
			e1_tube_info = e1_tube_init();
			/* 人体红外传感器初始化 */
			s7_ir_info = s7_ir_init();
			while(1)
			{	
				/* 读取人体红外传感器感应状态 */
				status = s7_ir_status_get(s7_ir_info);
				/* 如果检测到人体，在数码管上显示"1111" */
				if(status)
				{
					e1_tube_str_set(e1_tube_info, "1111");			
				}
				/* 如果没有检测到人体，在数码管上显示"----" */
				else
				{
					e1_tube_str_set(e1_tube_info, "----");				
				}
				/* 延时100ms */
				delay_ms(100);
			}
		}
		#endif
		
		


		
		

/* 实现温度测量及显示(U1、E1、S8) */
#if 0
#include <stdio.h>
#include "delay.h"
#include "e1.h"
#include "s8.h"
int main()
{
	char buf[10] = {0};
	s8_ths_t ths_value;
	/* 数码管初始化 */
	e1_tube_info = e1_tube_init();
	/* 温湿度传感器初始化 */
	s8_ths_info = s8_ths_init();
	while(1)
	{	
		/* 读取温湿度传感器测量结果（摄氏度、百分比） */
		ths_value = s8_ths_value_get(s8_ths_info);
		/* 将温度测量结果转换为字符串并写入buf */
		sprintf(buf, "%.1fC", ths_value.temp);
		/* 在数码管上显示buf中的字符串 */
		e1_tube_str_set(e1_tube_info, buf);
		/* 延时100ms */
		delay_ms(100);
	}
}
#endif

	
	
	
	
	
	
	
	
	
	
	
	
