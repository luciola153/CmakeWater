#include "system.h"

//定时器节点控制
uint32_t time;    //系统运行时间，单位：10ms
uint16_t Time_100ms; //100ms定时器
uint16_t Time_1s;    //1s定时器
uint16_t Time_1s_count;   //1s计数器
uint32_t time_b;

//OLED更新标志位
uint8_t OLED_Update_flag;   //OLED更新标志位
uint8_t OLED_Update_time;   //OLED更新计时器

//状态跳转参数
uint8_t run_mode;             //运行模式选择

//TOF测距参数
uint16_t distance;

// Modbus陀螺仪角度参数（USART2）
// *_raw : 原始16位有符号值，拼接方式为 (H << 8) | L
// *_deg : 换算后角度，单位°，范围约为 -180~180
// gyro_angle_valid : 1=收到并解析到一帧有效角度数据
volatile int16_t gyro_roll_raw = 0;
volatile int16_t gyro_pitch_raw = 0;
volatile int16_t gyro_yaw_raw = 0;
volatile float gyro_roll_deg = 0.0f;
volatile float gyro_pitch_deg = 0.0f;
volatile float gyro_yaw_deg = 0.0f;
volatile uint8_t gyro_angle_valid = 0;



u32 myabs(long int a)
{
    u32 temp;
    if (a < 0)
    {
        temp = -a;
    }
    else
    {
        temp = a;
    }
    return temp;
}
/*
int fputc(int ch, FILE *f)
{
		uint8_t temp[1] = {ch};
		HAL_UART_Transmit(&huart3, temp, 1, 2);//huart1??????????	HAL_MAX_DELAY
		return ch;
}
*/
// 使用寄存器方式发送字符，避免 printf 阻塞过久
#include <stdio.h>

// 重定向 printf 到 USART3
int fputc(int ch, FILE *f) 
{
    // 等待串口发送缓冲区空
    while((USART3->SR & USART_SR_TXE) == 0);
    USART3->DR = (ch & 0xFF);
    return ch;
}

int fgetc(FILE *f)
 {
    // 等待串口接收缓冲区非空
    while((USART3->SR & USART_SR_RXNE) == 0);
    return (int)USART3->DR;
}

