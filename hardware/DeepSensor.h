#ifndef __DEEP_SENSOR_H__
#define __DEEP_SENSOR_H__

#include "stm32f4xx_hal.h"
#include <stdint.h>

// MS5837-02BA I2C 7-bit地址（PS接GND=0x76，PS接VDD=0x77）
#define MS5837_ADDR_7BIT            0x76u

// 命令字（详见数据手册）
#define MS5837_CMD_RESET            0x1Eu
#define MS5837_CMD_ADC_READ         0x00u
#define MS5837_CMD_PROM_READ_BASE   0xA0u
#define MS5837_CMD_CONV_D1_OSR4096  0x48u
#define MS5837_CMD_CONV_D2_OSR4096  0x58u

// 传感器初始化：复位 + 读取C1~C6校准系数
HAL_StatusTypeDef DeepSensor_Init(void);
// 10ms节拍调用的非阻塞任务函数：完成D2/D1转换读取和补偿计算
void DeepSensor_Task10ms(void);
// 初始化状态查询：1=已就绪，0=未就绪
uint8_t DeepSensor_IsReady(void);

#endif
