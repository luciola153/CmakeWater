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
//#include "app_main.h"
#include "delay.h"
//#include "timer.h"
//#include "task_execute.h"
//#include "bsp_motor.h"
#include "uart_execute.h"
//#include "Emm_V5.h"
//#include "pressADC.h"
//#include "normal_motor.h"
//#include "TOF.h"

#include <stdio.h> 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stdarg.h"
#include "stdbool.h"  

extern uint32_t time;                //ϵͳ����ʱ�䣬��λ��10ms
extern uint16_t Time_100ms;          //100ms��ʱ��
extern uint16_t Time_1s;             //1s��ʱ��
extern uint8_t OLED_Update_flag;   //OLED���±�־λ
extern uint8_t OLED_Update_time;   //OLED���¼�ʱ��
extern uint8_t run_mode;             //����ģʽѡ��
extern uint16_t distance;



u32 myabs(long int a);
int fputc(int ch, FILE *f);
int fgetc(FILE *f);
#endif 
