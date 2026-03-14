// UART_NVIC.h
#ifndef __UART_NVIC_H__
#define __UART_NVIC_H__

#include "uart_execute.h"
#include "usart.h"

#define RX_BUFFER_SIZE 256

typedef struct
{
    uint8_t Rx[RX_BUFFER_SIZE];         // �� ���������� timer.c �ж�
    uint8_t Rx2[RX_BUFFER_SIZE];        // ���ջ��棨DMA �����ã�
    uint8_t number;                     // ���ݾɴ���
    uint16_t number_finall;             // ʵ��֡����
    volatile uint8_t rx_finall_flag;    // ֡��ɱ�־
    volatile uint8_t rx_finall_protect; // ���ظ��ύ����
} Serial_RxPacket;

// ֡״̬��
typedef enum
{
    WAIT_FRAME_HEAD = 0,
    WAIT_FRAME_DATA
} FrameState;

// �ⲿ������ÿ�����ڶ���
extern Serial_RxPacket RXdata[4];   // USART1
extern Serial_RxPacket RXdata_2[4]; // USART2
extern Serial_RxPacket RXdata_3[4]; // USART3
extern Serial_RxPacket RXdata_4[4]; // UART4
extern Serial_RxPacket RXdata_5[4]; // UART5
extern Serial_RxPacket RXdata_6[4]; // USART6

extern FrameState uart1_frame_state;
extern FrameState uart2_frame_state;
extern FrameState uart3_frame_state;
extern FrameState uart4_frame_state;
extern FrameState uart5_frame_state;
extern FrameState uart6_frame_state;

// DMA ���ջ�����
extern uint8_t dma_rx_buf1[RX_BUFFER_SIZE];
extern uint8_t dma_rx_buf2[RX_BUFFER_SIZE];
extern uint8_t dma_rx_buf3[RX_BUFFER_SIZE];
extern uint8_t dma_rx_buf4[RX_BUFFER_SIZE];
extern uint8_t dma_rx_buf5[RX_BUFFER_SIZE];
extern uint8_t dma_rx_buf6[RX_BUFFER_SIZE];

// 函数声明
void UART_DMA_Init(void);
void UART_IDLE_Callback(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size);
void Gyro_Modbus_RequestXYZ(void);
void Gyro_Modbus_Trigger10ms(void);
void Gyro_Modbus_Poll(void);
void parse_uart_frames(void);
void main_loop(void);

#endif
