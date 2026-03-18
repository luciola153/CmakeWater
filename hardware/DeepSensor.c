#include "DeepSensor.h"
#include "system.h"
#include "i2c.h"

typedef enum
{
    DS_STATE_IDLE = 0, // 空闲态：等待启动下一轮测量
    DS_STATE_WAIT_D2,  // 已发温度转换命令，等待转换完成后读D2
    DS_STATE_WAIT_D1   // 已发压力转换命令，等待转换完成后读D1
} DeepSensorState_t;

static uint16_t s_prom_c[7] = {0}; // PROM系数，使用索引1~6对应C1~C6
static uint32_t s_d1_raw = 0;       // 原始压力ADC值（24bit）
static uint32_t s_d2_raw = 0;       // 原始温度ADC值（24bit）
static uint8_t s_inited = 0;        // 初始化完成标志
static DeepSensorState_t s_state = DS_STATE_IDLE;
static uint32_t s_deadline_tick_10ms = 0;   // 当前阶段最早可读取的tick
static uint32_t s_last_cycle_tick_10ms = 0; // 上一次完整测量结束tick

static HAL_StatusTypeDef ds_write_cmd(uint8_t cmd)
{
    // HAL的设备地址参数需要左移1位（8-bit地址格式）
    return HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(MS5837_ADDR_7BIT << 1), &cmd, 1, 50);
}

static HAL_StatusTypeDef ds_read_bytes(uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(MS5837_ADDR_7BIT << 1), buf, len, 50);
}

static HAL_StatusTypeDef ds_read_prom_coeff(uint8_t idx, uint16_t *out_coeff)
{
    // PROM地址命令：0xA0 + (idx * 2)
    uint8_t cmd = (uint8_t)(MS5837_CMD_PROM_READ_BASE + (idx << 1));
    uint8_t buf[2] = {0};
    HAL_StatusTypeDef ret = ds_write_cmd(cmd);
    if (ret != HAL_OK)
    {
        return ret;
    }
    ret = ds_read_bytes(buf, 2);
    if (ret != HAL_OK)
    {
        return ret;
    }
    *out_coeff = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    return HAL_OK;
}

static HAL_StatusTypeDef ds_read_adc24(uint32_t *out_adc)
{
    uint8_t cmd = MS5837_CMD_ADC_READ;
    uint8_t buf[3] = {0};
    HAL_StatusTypeDef ret = ds_write_cmd(cmd);
    if (ret != HAL_OK)
    {
        return ret;
    }
    ret = ds_read_bytes(buf, 3);
    if (ret != HAL_OK)
    {
        return ret;
    }
    *out_adc = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    return HAL_OK;
}

static void ds_calculate(void)
{
    // MS5837-02BA补偿公式（TE Rev A8，page 7/8）
    // 注意：中间变量用int64_t，避免乘法溢出。
    int64_t dT;
    int64_t TEMP;
    int64_t OFF;
    int64_t SENS;
    int64_t Ti = 0;
    int64_t OFFi = 0;
    int64_t SENSi = 0;

    dT = (int64_t)s_d2_raw - ((int64_t)s_prom_c[5] << 8);
    TEMP = 2000 + ((dT * s_prom_c[6]) / 8388608LL);

    // 一阶补偿：
    // OFF  = C2*2^17 + C4*dT/2^6
    // SENS = C1*2^16 + C3*dT/2^7
    OFF = ((int64_t)s_prom_c[2] << 17) + ((dT * s_prom_c[4]) >> 6);
    SENS = ((int64_t)s_prom_c[1] << 16) + ((dT * s_prom_c[3]) >> 7);

    // 二阶补偿（低温/高温精度优化）
    if (TEMP < 2000)
    {
        int64_t t = TEMP - 2000;
        Ti = (11 * dT * dT) >> 35;
        OFFi = (31 * t * t) >> 3;
        SENSi = (63 * t * t) >> 5;

        if (TEMP < -1500)
        {
            int64_t t2 = TEMP + 1500;
            OFFi += 17 * t2 * t2;
            SENSi += 9 * t2 * t2;
        }
    }
    else
    {
        int64_t t = TEMP - 2000;
        Ti = (3 * dT * dT) >> 33;
        OFFi = (3 * t * t) >> 1;
        SENSi = (5 * t * t) >> 3;
    }

    {
        // 应用二阶补偿后的TEMP/OFF/SENS
        TEMP -= Ti;
        OFF -= OFFi;
        SENS -= SENSi;

        // 手册中的P单位为0.01mbar：
        // P = (D1*SENS/2^21 - OFF) / 2^15
        int64_t p_0p01mbar = ((((int64_t)s_d1_raw * SENS) >> 21) - OFF) >> 15;
        ms5837_pressure_mbar = (float)p_0p01mbar / 100.0f;
    }

    // TEMP单位为0.01°C
    ms5837_temperature_c = (float)TEMP / 100.0f;

    // 深度估算（淡水）：
    // depth(m) = (P - P0) * 100 / (rho * g), P单位为mbar
    ms5837_depth_m = ((ms5837_pressure_mbar - 1013.25f) * 100.0f) / (997.0f * 9.80665f);
    ms5837_data_valid = 1; // 置位表示本轮结果有效
}

HAL_StatusTypeDef DeepSensor_Init(void)
{
    HAL_StatusTypeDef ret;

    // 上电初始化流程：复位 -> 等待 -> 读取PROM系数
    ret = ds_write_cmd(MS5837_CMD_RESET);
    if (ret != HAL_OK)
    {
        return ret;
    }
    delay_ms(3);

    for (uint8_t i = 1; i <= 6; i++)
    {
        ret = ds_read_prom_coeff(i, &s_prom_c[i]);
        if (ret != HAL_OK)
        {
            return ret;
        }
    }

    s_state = DS_STATE_IDLE;
    s_last_cycle_tick_10ms = time;
    s_inited = 1;
    return HAL_OK;
}

void DeepSensor_Task10ms(void)
{
    HAL_StatusTypeDef ret;

    if (!s_inited)
    {
        return;
    }

    switch (s_state)
    {
    case DS_STATE_IDLE:
        // 每100ms启动一轮完整测量
        if ((uint32_t)(time - s_last_cycle_tick_10ms) >= 10u)
        {
            ret = ds_write_cmd(MS5837_CMD_CONV_D2_OSR4096);
            if (ret == HAL_OK)
            {
                s_deadline_tick_10ms = time + 1u; // OSR4096转换时间约8.2ms，留足10ms
                s_state = DS_STATE_WAIT_D2;
            }
        }
        break;

    case DS_STATE_WAIT_D2:
        if ((int32_t)(time - s_deadline_tick_10ms) >= 0)
        {
            ret = ds_read_adc24(&s_d2_raw);
            if (ret == HAL_OK)
            {
                ret = ds_write_cmd(MS5837_CMD_CONV_D1_OSR4096);
                if (ret == HAL_OK)
                {
                    s_deadline_tick_10ms = time + 1u;
                    s_state = DS_STATE_WAIT_D1;
                }
                else
                {
                    s_state = DS_STATE_IDLE;
                }
            }
            else
            {
                s_state = DS_STATE_IDLE;
            }
        }
        break;

    case DS_STATE_WAIT_D1:
        if ((int32_t)(time - s_deadline_tick_10ms) >= 0)
        {
            ret = ds_read_adc24(&s_d1_raw);
            if (ret == HAL_OK)
            {
                ds_calculate();
            }
            s_last_cycle_tick_10ms = time;
            s_state = DS_STATE_IDLE;
        }
        break;

    default:
        s_state = DS_STATE_IDLE;
        break;
    }
}

uint8_t DeepSensor_IsReady(void)
{
    return s_inited;
}
