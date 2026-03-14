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

static uint8_t usart2_temp_buf[8];
static uint16_t usart2_index = 0;
static uint8_t usart2_frame_valid = 0;

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

// 解析固定帧头 + 固定长度的协议，适用于 Modbus 设备
static void parse_fixed_head_fixed_len(
    uint8_t byte,
    uint8_t *temp_buf,          // 临时接收缓冲（最长 7 字节）
    uint16_t *index,            // 当前已接收字节数
    uint8_t *frame_valid,       // 帧是否有效标志（根据前 3 字节判断）
    Serial_RxPacket *pkt,
    int idx,
    uint16_t fixed_len)         // 固定帧长度
{
    if (*index == 0)
    {
        // 等待帧头 0x01
        if (byte == 0x01)
        {
            temp_buf[0] = byte;
            *index = 1;
            *frame_valid = 0; // 标记为无效帧
        }
        // 其他字节丢弃
    }
    else
    {
        // 已经收到帧头，继续接收
        if (*index < fixed_len)
        {
            temp_buf[*index] = byte;
            (*index)++;

            // 收到第 3 个字节时判断是否为合法 Modbus 帧
            if (*index == 3)
            {
                if (temp_buf[1] == 0x03 && temp_buf[2] == 0x02)
                {
                    *frame_valid = 1; // 标记为有效帧
                }
                else
                {
                    *frame_valid = 0; // 非法帧，后续仍然接收但标记无效
                }
            }

            // 接收满 fixed_len 字节
            if (*index >= fixed_len)
            {
                if (*frame_valid)
                {
                    // 只解析距离值（第 4、5 字节）
                    distance = (temp_buf[3] << 8) | temp_buf[4];
                    memcpy(pkt[idx].Rx2, temp_buf, fixed_len);
                    pkt[idx].number_finall = fixed_len;
                    pkt[idx].rx_finall_flag = 1;
                }

                // 重置状态机
                *index = 0;
                *frame_valid = 0;
            }
        }
        else
        {
            // 长度溢出，丢弃本帧
            *index = 0;
            *frame_valid = 0;
        }
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
            parse_fixed_head_fixed_len(
				byte,
				usart2_temp_buf, &usart2_index, &usart2_frame_valid,
				RXdata_2, 1,
				7); // Modbus 固定帧长 7 字节
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
    parse_uart_frames();
    process_uart_data(); // 在uart_execute.c 实现
}
