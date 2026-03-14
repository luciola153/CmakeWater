#include "motor.h"
#include "system.h"
#include "tim.h"  

void motor_init(void)
{
    HAL_TIM_PWM_Start(IN1_TIM, TIM_CHANNEL_1);
    TIM1->CCR1 = 1500;
}

//1500静止，小于1500反转，大于1500正转
void motor_pwm_set(uint8_t number, uint16_t duty)
{
    switch (number)
    {
    case 1:
        TIM1->CCR1 = duty;
        break;
    
    default:
        break;
    }
     // 直接设置占空比寄存器
}