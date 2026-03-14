#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f4xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"

/*PWM通道定义*/
#define IN1_TIM                               &htim1
#define MOTOR1_PUL_CHAANNEL                   TIM_CHANNEL_1
#define MOTOR1_PUL_TIM                        TIM1


void motor_init(void);
void motor_pwm_set(uint8_t number, uint16_t duty);


#endif



