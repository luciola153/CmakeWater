#include "APP_MAIN.h"
#include "system.h"

void APP_Init(void)        // 状态机初始化
{
	UART_DMA_Init();
    timer_init();
    OLED_Init();
    motor_init();
    motor_pwm_set(1, 1500);
    delay_ms(3000);


}

void APP_MAIN(void)        // 状态机主循环
{
    OLED_Show();
    test_all_task();
	main_loop();
    
   
}

void test_all_task(void)   // 任务切换执行
{
    switch (run_mode)
    {
    case 0:
        run_mode_zero();
        break;
    
    case 1:
        run_mode_one();
        break;

    case 2:
        run_mode_two();
        break;

    case 3:
        run_mode_three();
        break;
        
    case 4:
        run_mode_four();
        break;
        
    case 5:
        run_mode_five();
        break;

    case 6:
        run_mode_six();
        break;

    }
}

// OLED 显示更新
void OLED_Show(void)
{
    if(OLED_Update_flag == 1)
    {
        OLED_Clear();
        if (gyro_angle_valid)
        {
            OLED_ShowString(0, 0, "X:", OLED_8X16);
            OLED_ShowFloatNum(16, 0, gyro_roll_deg, 3, 2, OLED_8X16);

            OLED_ShowString(0, 16, "Y:", OLED_8X16);
            OLED_ShowFloatNum(16, 16, gyro_pitch_deg, 3, 2, OLED_8X16);

            OLED_ShowString(0, 32, "Z:", OLED_8X16);
            OLED_ShowFloatNum(16, 32, gyro_yaw_deg, 3, 2, OLED_8X16);
        }
        else
        {
            OLED_ShowString(0, 0, "GYRO WAIT...", OLED_8X16);
        }
        OLED_Update();
        OLED_Update_flag = 0;
    }

}


