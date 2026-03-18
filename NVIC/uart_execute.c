#include "system.h"
#include "uart_execute.h"
#include "UART_NVIC.h"
#include "usart.h"

static void split_fixed(float v, uint32_t scale, int32_t *ipart, uint32_t *fpart, char *sign)
{
    int32_t scaled = (int32_t)(v * (float)scale);
    if (scaled < 0)
    {
        *sign = '-';
        scaled = -scaled;
    }
    else
    {
        *sign = '+';
    }
    *ipart = scaled / (int32_t)scale;
    *fpart = (uint32_t)(scaled % (int32_t)scale);
}

static void deepsensor_uart1_report_task(void)
{
    static uint32_t last_report_tick_10ms = 0;
    char tx_buf[128];
    int n;
    int32_t p_i, t_i, d_i;
    uint32_t p_f, t_f, d_f;
    char p_s, t_s, d_s;

    if (!ms5837_data_valid)
    {
        return;
    }

    // ?500ms???USART1?????????????????
    if ((uint32_t)(time - last_report_tick_10ms) < 50u)
    {
        return;
    }
    last_report_tick_10ms = time;

    // Avoid float-format printf dependency by manual fixed-point split.
    split_fixed(ms5837_pressure_mbar, 100u, &p_i, &p_f, &p_s);   // 2 decimals
    split_fixed(ms5837_temperature_c, 100u, &t_i, &t_f, &t_s);   // 2 decimals
    split_fixed(ms5837_depth_m, 1000u, &d_i, &d_f, &d_s);        // 3 decimals

    n = snprintf(tx_buf, sizeof(tx_buf),
                 "MS5837 P=%c%ld.%02lu mbar T=%c%ld.%02lu C D=%c%ld.%03lu m\r\n",
                 p_s, (long)p_i, (unsigned long)p_f,
                 t_s, (long)t_i, (unsigned long)t_f,
                 d_s, (long)d_i, (unsigned long)d_f);
    if (n > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, (uint16_t)n, 50);
    }
}

// ?????UART???????????
void process_uart_data(void)
{
    deepsensor_uart1_report_task();

    // USART1
    if (RXdata[1].rx_finall_flag && !RXdata[1].rx_finall_protect)
    {
        RXdata[1].rx_finall_protect = 1;
        HAL_UART_Transmit_DMA(&huart1, RXdata[1].Rx2, RXdata[1].number_finall);
    }

    // USART2
    if (RXdata_2[1].rx_finall_flag && !RXdata_2[1].rx_finall_protect)
    {
        // USART2????Modbus????????????????????????????????
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

    // UART4: ?????????§¿?ï…???????????????? UART_NVIC.c ?????
    // ?????????¦Ë?????????????
    
    // UART5: ?????+???????§¿?ï…???????
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

// DMA ?????????????¦Ë?????????
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
    // UART4 ??¦Ä??? DMA ??????
}
