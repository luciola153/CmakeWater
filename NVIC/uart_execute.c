#include "system.h"
#include "uart_execute.h"
#include "UART_NVIC.h"
#include "usart.h"

// 在处理UART数据，循环执行
void process_uart_data(void)
{
    // USART1
    if (RXdata[1].rx_finall_flag && !RXdata[1].rx_finall_protect)
    {
        RXdata[1].rx_finall_protect = 1;
        HAL_UART_Transmit_DMA(&huart1, RXdata[1].Rx2, RXdata[1].number_finall);
    }

    // USART2
    if (RXdata_2[1].rx_finall_flag && !RXdata_2[1].rx_finall_protect)
    {
        // USART2用于Modbus读取陀螺仪：这里只标记已处理，不做回传
        RXdata_2[1].rx_finall_protect = 1;
        RXdata_2[1].rx_finall_flag = 0;
        RXdata_2[1].rx_finall_protect = 0;
    }

    // USART3
    if (RXdata_3[1].rx_finall_flag && !RXdata_3[1].rx_finall_protect)
    {
        RXdata_3[1].rx_finall_protect = 1;
        HAL_UART_Transmit_DMA(&huart3, RXdata_3[1].Rx2, RXdata_3[1].number_finall);
    }

    // UART4: 预定义特殊协议，用于停止读取数据，在 UART_NVIC.c 中实现
    // 需要根据标志位在此时插入处理逻辑
    
    // UART5: 固定帧头+固定长度协议，发送响应
    if (RXdata_5[1].rx_finall_flag && !RXdata_5[1].rx_finall_protect)
    {
        RXdata_5[1].rx_finall_protect = 1;
       HAL_UART_Transmit_DMA(&huart5, RXdata_5[1].Rx2, RXdata_5[1].number_finall);
       
    }

    // USART6
    if (RXdata_6[1].rx_finall_flag && !RXdata_6[1].rx_finall_protect)
    {
        RXdata_6[1].rx_finall_protect = 1;
        HAL_UART_Transmit_DMA(&huart6, RXdata_6[1].Rx2, RXdata_6[1].number_finall);
    }
}

// DMA 完成发送后，处理标志位，执行下一帧
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        RXdata[1].rx_finall_flag = 0;
        RXdata[1].rx_finall_protect = 0;
    }
    else if (huart->Instance == USART2)
    {
        RXdata_2[1].rx_finall_flag = 0;
        RXdata_2[1].rx_finall_protect = 0;
    }
    else if (huart->Instance == USART3)
    {
        RXdata_3[1].rx_finall_flag = 0;
        RXdata_3[1].rx_finall_protect = 0;
    }
    else if (huart->Instance == UART5)
    {
        RXdata_5[1].rx_finall_flag = 0;
        RXdata_5[1].rx_finall_protect = 0;
    }
    else if (huart->Instance == USART6)
    {
        RXdata_6[1].rx_finall_flag = 0;
        RXdata_6[1].rx_finall_protect = 0;
    }
    // UART4 目前未使用 DMA 完成发送
}
