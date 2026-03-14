#include "system.h"

//定时器节点控制
uint32_t time;    //系统运行时间，单位：10ms
uint16_t Time_100ms; //100ms定时器
uint16_t Time_1s;    //1s定时器
uint16_t Time_1s_count;   //1s计数器
uint32_t time_b;

//OLED更新标志位
uint8_t OLED_Update_flag;   //OLED更新标志位
uint8_t OLED_Update_time;   //OLED更新计时器

//状态跳转参数
uint8_t run_mode;             //运行模式选择

//TOF测距参数
uint16_t distance;



u32 myabs(long int a)
{ 		   
	  u32 temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}
/*
int fputc(int ch, FILE *f)
{
		uint8_t temp[1] = {ch};
		HAL_UART_Transmit(&huart3, temp, 1, 2);//huart1??????????	HAL_MAX_DELAY
		return ch;
}
*/
// ����ʵ���������������printf�Ῠ��
#include <stdio.h>

// �ض���printf������
int fputc(int ch, FILE *f) 
{
    // �ȴ����ڷ���׼����
    while((USART3->SR & USART_SR_TXE) == 0);
    USART3->DR = (ch & 0xFF);
    return ch;
}

int fgetc(FILE *f)
 {
    // �ȴ����ڽ�������
    while((USART3->SR & USART_SR_RXNE) == 0);
    return (int)USART3->DR;
}

