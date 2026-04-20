#ifndef __USART_H
#define __USART_H

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MAX_RX_BUFF            1024
#define MAX_RX_BLOCK           256                              // 每一个数据帧最大的数值
#define BLOCK_NUMBER           3                                // block number
#define USART0_DATA_ADDRESS    ((uint32_t)&USART_DATA(USART0))  // USART_DATA地址

typedef struct{
    volatile uint8_t * volatile start;   // 数据块的起始位置
    volatile uint8_t * volatile stop;    // 数据块的终止位置，即此次接收到的数据长度
}UCB_RX_BLOCK_ptr;   // 8 byte,每个指针都是4byte    start与stop指向的地址易变

typedef struct{
    volatile uint16_t Data_Number;  // u0_rx_buff中存储的数据总数   Data_Number 易变
    volatile UCB_RX_BLOCK_ptr Block[BLOCK_NUMBER];  // u0_rx_buff 分为 BLOCK_NUMBER 个块，每个块存储最多256byte数据，MAX_RX_BLOCK
    volatile UCB_RX_BLOCK_ptr *volatile In;  
    volatile UCB_RX_BLOCK_ptr *volatile Out;    // 指针变量本身指向的地址易变
    volatile UCB_RX_BLOCK_ptr *volatile End;    // 始终指向block的最后一位
}UCB_t;


void USART0_init(uint32_t baudval);
void u0_printf(char *format, ...);
bool receive_data(void);
uint16_t receive_string(uint8_t *buffer, uint16_t max_len);
void DMA0_Init(void);
void Block_Init(void);
void data_handle(void);

extern uint16_t u0_rx_data;
extern uint8_t u0_rx_buff[MAX_RX_BUFF];
extern UCB_t block_control_t;

#endif
