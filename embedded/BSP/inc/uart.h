#ifndef __UART_H__
#define __UART_H__

#include "gd32f4xx.h"
#define UART_RECVBUF_MAXLEN 128

typedef enum
{
    STATE_RX_IDLE,		//����           
    STATE_RX_RECEIVING,	//���ݽ�����
    STATE_RX_ERROR,		//���մ���
    STATE_RX_RECEIVED	//���ݽ������
}UartRecvState_t;

//ʹ�õ�ͨ�Ŵ���
typedef enum{
	COM_USART0,
	COM_USART1,
	COM_USART2,
	COM_UART4,
	COM_USART5,
	COM_UART6,
	COM_UART7,
	COM_USART_NUMMAX
}Com_Usrat_Used_t;



void uart_init(uint32_t usart_periph);
void timer3_init(void);
//void delay_ms(uint16_t mstime);
uint16_t uart_send_bytes(uint32_t usart_periph, uint8_t *data, uint16_t len);
uint16_t uart_rece_bytes(uint32_t usart_periph, uint8_t *data, uint16_t len, uint16_t timeout);




#endif
