#include "system.h"
#include "uart_execute.h"
#include "UART_NVIC.h"
#include "usart.h"

// 处理所有串口数据（主循环调用）
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
        PCout(4) = 1;
        RXdata_2[1].rx_finall_protect = 1;
        HAL_UART_Transmit_DMA(&huart2, RXdata_2[1].Rx2, RXdata_2[1].number_finall);
        PCout(4) = 0;
    }

    // USART3
    if (RXdata_3[1].rx_finall_flag && !RXdata_3[1].rx_finall_protect)
    {
        RXdata_3[1].rx_finall_protect = 1;
        HAL_UART_Transmit_DMA(&huart3, RXdata_3[1].Rx2, RXdata_3[1].number_finall);
    }

    // UART4: 接收专用，不发送（已在 UART_NVIC.c 中处理 zdt_stop_read）
    // 可选择在此清标志（如果不需要重入）
    
    // UART5: 无头帧，收到即发（防重入）
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

// DMA 发送完成回调：清标志，允许下一次发送
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
    // UART4 不发送，无需处理
}
