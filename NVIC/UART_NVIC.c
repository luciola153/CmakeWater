#include "UART_NVIC.h"
#include "system.h"

// 各串口接收的数据包（每个通道一个缓冲）
Serial_RxPacket RXdata[4] = {0};
Serial_RxPacket RXdata_2[4] = {0};
Serial_RxPacket RXdata_3[4] = {0};
Serial_RxPacket RXdata_4[4] = {0};
Serial_RxPacket RXdata_5[4] = {0};
Serial_RxPacket RXdata_6[4] = {0};

// 各串口当前帧状态
FrameState uart1_frame_state = WAIT_FRAME_HEAD;
FrameState uart2_frame_state = WAIT_FRAME_HEAD;
FrameState uart3_frame_state = WAIT_FRAME_HEAD;
FrameState uart4_frame_state = WAIT_FRAME_HEAD;
FrameState uart5_frame_state = WAIT_FRAME_HEAD;
FrameState uart6_frame_state = WAIT_FRAME_HEAD;

// 临时帧缓冲区
static uint8_t uart1_temp_frame_buffer[RX_BUFFER_SIZE];
static uint8_t uart2_temp_frame_buffer[RX_BUFFER_SIZE];
static uint8_t uart3_temp_frame_buffer[RX_BUFFER_SIZE];
static uint8_t uart4_temp_frame_buffer[RX_BUFFER_SIZE];
static uint8_t uart5_temp_frame_buffer[RX_BUFFER_SIZE];
static uint8_t uart6_temp_frame_buffer[RX_BUFFER_SIZE];

// 当前帧索引（记录已接收字节数）
static uint16_t uart1_current_frame_index = 0;
static uint16_t uart2_current_frame_index = 0;
static uint16_t uart3_current_frame_index = 0;
static uint16_t uart4_current_frame_index = 0;
static uint16_t uart5_current_frame_index = 0;
static uint16_t uart6_current_frame_index = 0;

// DMA 接收缓冲区
uint8_t dma_rx_buf1[RX_BUFFER_SIZE];
uint8_t dma_rx_buf2[RX_BUFFER_SIZE];
uint8_t dma_rx_buf3[RX_BUFFER_SIZE];
uint8_t dma_rx_buf4[RX_BUFFER_SIZE];
uint8_t dma_rx_buf5[RX_BUFFER_SIZE];
uint8_t dma_rx_buf6[RX_BUFFER_SIZE];

static uint16_t usart2_expected_frame_len = 0;
static uint32_t usart2_query_tick_10ms = 0;
static uint8_t usart2_wait_reply = 0;
static volatile uint8_t usart2_query_due_flag = 0;
static const uint8_t gyro_modbus_query_cmd[8] = {0x50, 0x03, 0x00, 0x30, 0x00, 0x30, 0x48, 0x50};

// 初始化 DMA 接收
void UART_DMA_Init(void)
{

    HAL_UART_Receive_DMA(&huart1, dma_rx_buf1, RX_BUFFER_SIZE);
    HAL_UART_Receive_DMA(&huart2, dma_rx_buf2, RX_BUFFER_SIZE);
    HAL_UART_Receive_DMA(&huart3, dma_rx_buf3, RX_BUFFER_SIZE);
    HAL_UART_Receive_DMA(&huart4, dma_rx_buf4, RX_BUFFER_SIZE);
    HAL_UART_Receive_DMA(&huart5, dma_rx_buf5, RX_BUFFER_SIZE);
    HAL_UART_Receive_DMA(&huart6, dma_rx_buf6, RX_BUFFER_SIZE);

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE);
}

// 解析带帧头+帧尾的协议，适用于 USART1/3
// byte: 当前接收字节  head: 帧头  tail: 帧尾  temp_buf: 临时帧缓冲
// index: 当前帧长度  state: 帧状态  pkt: 接收包数组  idx: 使用的包下标
static void parse_with_head_tail(
    uint8_t byte, uint8_t head, uint8_t tail,
    uint8_t *temp_buf, uint16_t *index, FrameState *state,
    Serial_RxPacket *pkt, int idx)
{
    switch (*state)
    {
    case WAIT_FRAME_HEAD:
        if (byte == head)
        {
            temp_buf[0] = byte;
            *index = 1;
            *state = WAIT_FRAME_DATA;
        }
        break;
    case WAIT_FRAME_DATA:
        if (byte == head) // 防止连续多个帧头，只以最新的为准
        {
            temp_buf[0] = byte;
            *index = 1;
        }
        else if (byte == tail)
        {
            temp_buf[*index] = byte;
            (*index)++;
            memcpy(pkt[idx].Rx2, temp_buf, *index);
            pkt[idx].number_finall = *index;
            pkt[idx].rx_finall_flag = 1;
            *state = WAIT_FRAME_HEAD;
            *index = 0;
        }
        else
        {
            if (*index < RX_BUFFER_SIZE - 1)
            {
                temp_buf[*index] = byte;
                (*index)++;
            }
            else
            {
                *state = WAIT_FRAME_HEAD;
                *index = 0;
            }
        }
        break;
    }
}

// 解析无帧头、仅靠帧尾的协议，适用于 UART4/5/6
static void parse_no_head(
    uint8_t byte, uint8_t tail,
    uint8_t *temp_buf, uint16_t *index, FrameState *state,
    Serial_RxPacket *pkt, int idx)
{
    switch (*state)
    {
    case WAIT_FRAME_HEAD:
        temp_buf[0] = byte;
        *index = 1;
        if (byte == tail)
        {
            memcpy(pkt[idx].Rx2, temp_buf, 1);
            pkt[idx].number_finall = 1;
            pkt[idx].rx_finall_flag = 1;
            *state = WAIT_FRAME_HEAD;
            *index = 0;
        }
        else
        {
            *state = WAIT_FRAME_DATA;
        }
        break;
    case WAIT_FRAME_DATA:
        if (*index < RX_BUFFER_SIZE - 1)
        {
            temp_buf[*index] = byte;
            (*index)++;
            if (byte == tail)
            {
                memcpy(pkt[idx].Rx2, temp_buf, *index);
                pkt[idx].number_finall = *index;
                pkt[idx].rx_finall_flag = 1;
                *state = WAIT_FRAME_HEAD;
                *index = 0;
            }
        }
        else
        {
            *state = WAIT_FRAME_HEAD;
            *index = 0;
        }
        break;
    }
}

static uint16_t modbus_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static float gyro_raw_to_deg(int16_t raw)
{
    float deg = (float)raw * 180.0f / 32768.0f;
    if (deg > 180.0f)
    {
        deg -= 360.0f;
    }
    else if (deg < -180.0f)
    {
        deg += 360.0f;
    }
    return deg;
}

// 解析陀螺仪 Modbus 帧（地址0x50，功能码0x03，动态长度）
static void parse_gyro_modbus_stream(
    uint8_t byte,
    uint8_t *temp_buf,
    uint16_t *index,
    uint16_t *expected_len,
    Serial_RxPacket *pkt,
    int idx)
{
    if (*index == 0)
    {
        if (byte == 0x50)
        {
            temp_buf[0] = byte;
            *index = 1;
            *expected_len = 0;
        }
        return;
    }

    if (*index >= RX_BUFFER_SIZE)
    {
        *index = 0;
        *expected_len = 0;
        return;
    }

    temp_buf[*index] = byte;
    (*index)++;

    if (*index == 2 && temp_buf[1] != 0x03)
    {
        *index = 0;
        *expected_len = 0;
        return;
    }

    if (*index == 3)
    {
        uint8_t byte_count = temp_buf[2];
        uint16_t frame_len = (uint16_t)byte_count + 5u; // addr+func+byteCount+data+crc(2)
        if (frame_len > RX_BUFFER_SIZE || byte_count < 32u) // 至少要覆盖XYZ角度字节
        {
            *index = 0;
            *expected_len = 0;
        }
        else
        {
            *expected_len = frame_len;
        }
        return;
    }

    if (*expected_len > 0 && *index >= *expected_len)
    {
        uint16_t crc_recv = (uint16_t)temp_buf[*expected_len - 2] |
                            ((uint16_t)temp_buf[*expected_len - 1] << 8);
        uint16_t crc_calc = modbus_crc16(temp_buf, *expected_len - 2);

        if (crc_calc == crc_recv)
        {
            // 角度数据位于数据区第26~31字节（示例帧: 00 A3 FF CB 9B 30）
            const uint16_t base = 3; // 跳过 addr/func/bytecount
            int16_t roll_raw  = (int16_t)(((uint16_t)temp_buf[base + 26] << 8) | temp_buf[base + 27]);
            int16_t pitch_raw = (int16_t)(((uint16_t)temp_buf[base + 28] << 8) | temp_buf[base + 29]);
            int16_t yaw_raw   = (int16_t)(((uint16_t)temp_buf[base + 30] << 8) | temp_buf[base + 31]);

            gyro_roll_raw = roll_raw;
            gyro_pitch_raw = pitch_raw;
            gyro_yaw_raw = yaw_raw;
            gyro_roll_deg = gyro_raw_to_deg(roll_raw);
            gyro_pitch_deg = gyro_raw_to_deg(pitch_raw);
            gyro_yaw_deg = gyro_raw_to_deg(yaw_raw);
            gyro_angle_valid = 1;

            memcpy(pkt[idx].Rx2, temp_buf, *expected_len);
            pkt[idx].number_finall = *expected_len;
            pkt[idx].rx_finall_flag = 1;
            usart2_wait_reply = 0;
        }

        *index = 0;
        *expected_len = 0;
    }
}

void Gyro_Modbus_RequestXYZ(void)
{
    if (usart2_wait_reply)
    {
        return;
    }

    // RS485方向控制：拉高发送，发送完成后拉低回到接收
    PCout(4) = 1;
    if (HAL_UART_Transmit(&huart2, (uint8_t *)gyro_modbus_query_cmd, sizeof(gyro_modbus_query_cmd), 20) == HAL_OK)
    {
        usart2_wait_reply = 1;
        usart2_query_tick_10ms = time;
    }
    PCout(4) = 0;
}

void Gyro_Modbus_Trigger10ms(void)
{
    // 由10ms定时器节拍置位，请求下一次查询
    usart2_query_due_flag = 1;
}

void Gyro_Modbus_Poll(void)
{
    // 超时保护：约300ms未收到应答则允许重发
    if (usart2_wait_reply && (uint32_t)(time - usart2_query_tick_10ms) > 30u)
    {
        usart2_wait_reply = 0;
    }

    // 一发一收制：只有收到定时器触发，且当前不等待应答时才发送
    if (usart2_query_due_flag && !usart2_wait_reply)
    {
        usart2_query_due_flag = 0;
        Gyro_Modbus_RequestXYZ();
    }
}

// IDLE 中断回调接口，在 stm32f4xx_it.c 中调用
void UART_IDLE_Callback(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        uint8_t byte = buf[i];

        if (huart->Instance == USART1)
        {
            parse_with_head_tail(byte, 0x6C, 0x7F,
                                 uart1_temp_frame_buffer, &uart1_current_frame_index, &uart1_frame_state,
                                 RXdata, 1);
        }
        else if (huart->Instance == USART2)
        {
            parse_gyro_modbus_stream(
                byte,
                uart2_temp_frame_buffer,
                &uart2_current_frame_index,
                &usart2_expected_frame_len,
                RXdata_2, 1);
        }
        else if (huart->Instance == USART3)
        {
            parse_with_head_tail(byte, 0x7F, 0x6B,
                                 uart3_temp_frame_buffer, &uart3_current_frame_index, &uart3_frame_state,
                                 RXdata_3, 1);
        }
        else if (huart->Instance == UART4)
        {
            parse_no_head(byte, 0x6B,
                          uart4_temp_frame_buffer, &uart4_current_frame_index, &uart4_frame_state,
                          RXdata_4, 1);
            if (RXdata_4[1].rx_finall_flag)
            {
                // 把 Rx2 数据同步到 Rx
                memcpy(RXdata_4[1].Rx, RXdata_4[1].Rx2, RXdata_4[1].number_finall);
                RXdata_4[1].number = (uint8_t)RXdata_4[1].number_finall;
            }
        }
        else if (huart->Instance == UART5)
        {
            parse_no_head(byte, 0x6D, // 仅以帧尾 0x6D 作为结束，没有帧头
                          uart5_temp_frame_buffer, &uart5_current_frame_index, &uart5_frame_state,
                          RXdata_5, 1);
            if (RXdata_5[1].rx_finall_flag)
            {
                // 同步 Rx2 到 Rx，供业务层使用
                memcpy(RXdata_5[1].Rx, RXdata_5[1].Rx2, RXdata_5[1].number_finall);
                RXdata_5[1].number = (uint8_t)RXdata_5[1].number_finall;
            }
        }
        else if (huart->Instance == USART6)
        {
            parse_no_head(byte, 0x21,
                          uart6_temp_frame_buffer, &uart6_current_frame_index, &uart6_frame_state,
                          RXdata_6, 1);
            if (RXdata_6[1].rx_finall_flag)
            {
                memcpy(RXdata_6[1].Rx, RXdata_6[1].Rx2, RXdata_6[1].number_finall);
                RXdata_6[1].number = (uint8_t)RXdata_6[1].number_finall;
            }
        }
    }
}

void parse_uart_frames(void)
{
    // 预留：集中处理在 IDLE 回调中解析出的帧
}

void main_loop(void)
{
    Gyro_Modbus_Poll();
    parse_uart_frames();
    process_uart_data(); // 在uart_execute.c 实现
}
