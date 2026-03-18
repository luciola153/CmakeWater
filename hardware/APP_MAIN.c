#include "APP_MAIN.h"
#include "system.h"

void APP_Init(void)        // 状态机初始化
{
	UART_DMA_Init();
    timer_init();
    OLED_Init();
    // 深度传感器上电后先复位并读取PROM系数
    DeepSensor_Init();
    motor_init();
    motor_pwm_set(1, 1500);
    delay_ms(3000);


}

void APP_MAIN(void)        // 状态机主循环
{
    // 以10ms节拍驱动深度传感器状态机（非阻塞）
    DeepSensor_Task10ms();
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
        if (ms5837_data_valid)
        {
            // 主显示项：深度值
            OLED_ShowString(0, 0, "DEPTH(m):", OLED_8X16);
            OLED_ShowFloatNum(0, 16, ms5837_depth_m, 2, 3, OLED_8X16);

            // 辅助显示：压力和温度
            OLED_ShowString(0, 32, "P:", OLED_8X16);
            OLED_ShowFloatNum(16, 32, ms5837_pressure_mbar, 4, 2, OLED_8X16);
            OLED_ShowString(88, 32, "mb", OLED_8X16);

            OLED_ShowString(0, 48, "T:", OLED_8X16);
            OLED_ShowFloatNum(16, 48, ms5837_temperature_c, 2, 2, OLED_8X16);
            OLED_ShowString(80, 48, "C", OLED_8X16);
        }
        else
        {
            OLED_ShowString(0, 0, "MS5837 WAIT...", OLED_8X16);
        }
        OLED_Update();
        OLED_Update_flag = 0;
    }

}


