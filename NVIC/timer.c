#include "timer.h"
#include "system.h"

void timer_init(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == htim6.Instance)
    {
        // 10ms 对应的定时处理
        time++;
        Time_100ms++;
        Time_1s++;
        Gyro_Modbus_Trigger10ms();

        // 100ms 对应的定时处理
        if (Time_100ms >= 10)
        {
            Time_100ms = 0;
           
        }

        // 1s 对应的定时处理
        if (Time_1s >= 100)
        {
            Time_1s = 0;
            Time_1s_count++;
        }

        // OLED 刷新定时控制（20Hz）
        if(OLED_Update_flag != 1 && OLED_Update_time < 5)
        {
            OLED_Update_time++;
        }

        else if(OLED_Update_flag != 1 && OLED_Update_time >= 5)
        {
            OLED_Update_flag = 1;
            OLED_Update_time = 0;
        }

    }
   
}

