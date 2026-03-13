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
        //10ms��Ӧ����
        time++;
        Time_100ms++;
        Time_1s++;

        //100ms��Ӧ����
        if (Time_100ms >= 10)
        {
            Time_100ms = 0;
           
        }

        //1s��Ӧ����
        if (Time_1s >= 100)
        {
            Time_1s = 0;
            
        }

        //OLEDˢ��������(20Hz)
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

