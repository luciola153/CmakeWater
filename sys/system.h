#ifndef __SYSTEM_H
#define __SYSTEM_H

/*
 * Project include policy:
 * 1) system.h is an aggregate header intended for .c files.
 * 2) Do not include system.h from any other .h file.
 * 3) Header files should include only their direct dependencies.
 */

#define subdivision 16
#define PI 3.14159265358979323846f
#define Log 1

#include "stm32f4xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
//#include "adc.h"

#include "UART_NVIC.h"
#include "timer.h"
#include "sys.h"
#include "OLED.h"
#include "APP_MAIN.h"
#include "delay.h"
#include "task_execute.h"
#include "uart_execute.h"
#include "motor.h"

#include <stdio.h> 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stdarg.h"
#include "stdbool.h"  

extern uint32_t time;                //系统运行时间，单位：10ms
extern uint16_t Time_100ms;          //100ms定时器
extern uint16_t Time_1s;             //1s定时器
extern uint8_t OLED_Update_flag;   //OLED更新标志位
extern uint8_t OLED_Update_time;   //OLED更新计时器
extern uint8_t run_mode;             //运行模式选择
extern uint16_t distance;
extern uint16_t Time_1s_count;   //1s计数器
extern volatile int16_t gyro_roll_raw;   // X轴原始值（Roll[15:0]）
extern volatile int16_t gyro_pitch_raw;  // Y轴原始值（Pitch[15:0]）
extern volatile int16_t gyro_yaw_raw;    // Z轴原始值（Yaw[15:0]）
extern volatile float gyro_roll_deg;     // X轴角度，单位°，范围约 -180~180
extern volatile float gyro_pitch_deg;    // Y轴角度，单位°，范围约 -180~180
extern volatile float gyro_yaw_deg;      // Z轴角度，单位°，范围约 -180~180
extern volatile uint8_t gyro_angle_valid; // 角度数据有效标志：1=有新有效数据



u32 myabs(long int a);
int fputc(int ch, FILE *f);
int fgetc(FILE *f);
#endif 
