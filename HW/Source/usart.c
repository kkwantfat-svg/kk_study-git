#include "gd32f10x.h"
#include "gd32f10x_gpio.h"
#include "gd32f10x_usart.h"
#include "gd32f10x_dma.h"
#include "usart.h"
#include "gd32f10x_misc.h"

static uint8_t u0_tx_buff[256];   // 发送数据buff
uint16_t u0_rx_data;
uint8_t u0_rx_buff[MAX_RX_BUFF];  // 接收数据buff

UCB_t block_control_t;

void dma_enable(uint8_t flag);

/*!
    \brief      USART init
    \param[in]  baudval: baud rate value
    \param[out] none
    \retval     none
*/ 
void USART0_init(uint32_t baudval)
{
    rcu_periph_clock_enable(RCU_GPIOA);    // 使能时钟
    rcu_periph_clock_enable(RCU_USART0);   // 使能时钟

    // gpio 初始化
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);   // PA10  浮空输入
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);          // PA9   复用推挽输出

    // USART 初始化
    usart_deinit(USART0);  // deinit
    usart_baudrate_set(USART0, baudval); 
    /* configure USART parity function */
    usart_parity_config(USART0, USART_PM_NONE); 
    /* configure USART word length */
    usart_word_length_set(USART0, USART_WL_8BIT);
    /* configure USART stop bit length */
    usart_stop_bit_set(USART0, USART_STB_1BIT);

    dma_enable(1);
}

// 是否启用dma模式
void dma_enable(uint8_t flag)
{
    if(flag){
        // DMA 只是替代 CPU 完成“从数据寄存器取走数据”这一步，数据本身的接收工作仍然由 USART 硬件完成。因此，必须同时使能 USART 接收功能才能让数据进入系统。
        /* configure USART DMA for reception */
        usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
        /* configure USART DMA for transmission */
        usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
        /* configure USART transmitter */
        usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
        /* configure USART receiver */
        usart_receive_config(USART0, USART_RECEIVE_ENABLE);
        /* enable USART */
        usart_enable(USART0);

        usart_interrupt_enable(USART0, USART_INT_IDLE);   // 开启空闲中断
        nvic_irq_enable(USART0_IRQn, 0, 0);               // 使能中断，此处没分组，默认使用的是系统分组，NVIC_PRIGROUP_PRE2_SUB2
        Block_Init();
    }else{
        /* configure USART transmitter */
        usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
        /* configure USART receiver */
        usart_receive_config(USART0, USART_RECEIVE_ENABLE);
        /* enable USART */
        usart_enable(USART0);
    }
}

/*!
    \brief      DMA init
    \param[in]  none
    \param[out] none
    \retval     none
*/ 
void DMA0_Init(void)
{
    rcu_periph_clock_enable(RCU_DMA0);    // 使能时钟
    dma_parameter_struct dma_struct_init;

    dma_deinit(DMA0, DMA_CH4);  //  deinit
    /* initialize the parameters of DMA struct with the default values */
    dma_struct_para_init(&dma_struct_init);
    dma_struct_init.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_struct_init.memory_addr = (uint32_t)u0_rx_buff;
    dma_struct_init.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct_init.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_struct_init.number = MAX_RX_BLOCK + 1;                // 保证每个数据存储块接收到的数据都空余1个位置
    dma_struct_init.periph_addr = USART0_DATA_ADDRESS;
    dma_struct_init.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct_init.priority = DMA_PRIORITY_HIGH;
    dma_struct_init.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    /* initialize DMA channel */
    dma_init(DMA0, DMA_CH4, &dma_struct_init);

    /* disable DMA circulation mode */
    dma_circulation_disable(DMA0, DMA_CH4);  // 关闭循环模式
    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH4);
}

/********************************************************* 重写printf函数 **********************************************************/
void u0_printf(char *format, ...)
{
    va_list listdata;
    va_start(listdata, format);
    vsprintf((char *)u0_tx_buff, format, listdata);
    va_end(listdata);

    for(uint16_t i = 0; i < strlen((const char *)u0_tx_buff); i ++)
    {
        while(!usart_flag_get(USART0, USART_FLAG_TBE));  // 等待发送数据寄存器为空（缓冲区空了，可以进行发数据了）
        usart_data_transmit(USART0, u0_tx_buff[i]);   // 发送数据，并清除tbe标志位与置位的TC标志位
    }
    while(!usart_flag_get(USART0, USART_FLAG_TC));   // 等待发送完成，完成了读USART_STAT, 还差写USART_DATA才可以清除该标志位，下次在执行该函数时， usart_data_transmit(USART0, u0_tx_buff[i]); 会清除标志位TC
}

// 有数据时返回真，无数据时返回假，接收到的数据写在了 u0_rx_data 中，调用时先调用该函数判断是否接收到了数据，之后读 u0_rx_data 即可
bool receive_data(void)  // 该函数每读一次都会清空所接收到的数据
{
    if(usart_flag_get(USART0, USART_FLAG_RBNE))  // 获取rxne标志位,表示数据非空，有数据
    {
        u0_rx_data = usart_data_receive(USART0);     // 清除rxne标志位并获取数据
        return TRUE;
    }
    return FALSE;
}

// 接收一行数据，以 '\n' 结束，返回实际接收的字符数，阻塞函数，当无数据时会卡在此处
uint16_t receive_string(uint8_t *buffer, uint16_t max_len)
{
    uint16_t i = 0;
    while (i < max_len - 1) {          // 留一个位置给 '\0'
        while (!usart_flag_get(USART0, USART_FLAG_RBNE));  // 获取标志位
        uint8_t c = (uint8_t)usart_data_receive(USART0);   // 读数据，并清除标志位，只获取低八位数据，高八位数据会被截断，usart配置的数据宽度为8bit所以可以直接强转
        if (c == '\n') {               // 遇到换行符，结束接收
            buffer[i] = '\0';
            return i;
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';                  // 缓冲区满，强制结束
    return i;     // 返回的为数据的长度包括结尾 '\0'
}

/********************************************************* 环形缓冲区加空闲中断加DMA接收数据 **********************************************************/
/* buff控制块初始化 */
void Block_Init(void)
{
    // start 应该是在初始化时设置，而不是在每次中断中都重新设置当前块的 start。当前块的 start 应该在其被 In 指向时已经存在。
    // 设置所有块的 start 为缓冲区中对应的位置（以 MAX_RX_BLOCK 为步长）
    for (int i = 0; i < BLOCK_NUMBER; i++) {
        block_control_t.Block[i].start = &u0_rx_buff[i * MAX_RX_BLOCK];
        block_control_t.Block[i].stop  = NULL; // 初始无数据
    }
    block_control_t.Data_Number = 0;
    block_control_t.In  = block_control_t.Block;
    block_control_t.Out = block_control_t.Block;
    block_control_t.End = &block_control_t.Block[BLOCK_NUMBER - 1];
    
    // 第一个块已经由初始化设置好了 start
}

/******************** 中断服务函数 ******************/
void USART0_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
    {
        usart_data_receive(USART0); // 清空闲标志

        dma_channel_disable(DMA0, DMA_CH4);

        // 获取本次接收的字节数
        uint16_t remain = dma_transfer_number_get(DMA0, DMA_CH4);
        uint16_t recv_len = (MAX_RX_BLOCK + 1) - remain;
        
        if (recv_len > 0) {
            // 当前块（In 指向的块）已经存在 start，只需设置 stop
            block_control_t.In->stop = &u0_rx_buff[block_control_t.Data_Number + recv_len - 1];
            
            // 更新全局计数器
            block_control_t.Data_Number += recv_len;
            
            // 移动 In 指针到下一个块
            block_control_t.In++;
            if (block_control_t.In > block_control_t.End)
                block_control_t.In = block_control_t.Block;
            
            // 为下一个块设置 start（如果当前 Data_Number 已接近缓冲区末尾）
            // 注意：这里需要确保下一个块的 start 指向正确的缓冲区地址
            if ((MAX_RX_BUFF - block_control_t.Data_Number) >= MAX_RX_BLOCK) {
                // 剩余空间足够，start 继续递增
                block_control_t.In->start = &u0_rx_buff[block_control_t.Data_Number];
            } else {
                // 剩余空间不足，绕回缓冲区开头
                block_control_t.In->start = u0_rx_buff;
                block_control_t.Data_Number = 0;  // 重置计数器
            }
        }
        
        // 重新配置 DMA
        dma_transfer_number_config(DMA0, DMA_CH4, MAX_RX_BLOCK + 1);
        dma_memory_address_config(DMA0, DMA_CH4, (uint32_t)block_control_t.In->start);
        dma_channel_enable(DMA0, DMA_CH4);
    }
}

/******************** 数据接收处理 ******************/
void data_handle(void)
{
    while (block_control_t.Out != block_control_t.In)
    {
        // 确保当前块的 start 和 stop 都非空
        if (block_control_t.Out->start != NULL && block_control_t.Out->stop != NULL) {
            uint16_t len = block_control_t.Out->stop - block_control_t.Out->start + 1;
            u0_printf("********此次共接收 %d byte 数据***********\r\n", len);
            // 处理数据...
            for(uint16_t i = 0; i < len; i ++)
            {
                u0_printf("%c", block_control_t.Out->start[i]);
             }
        }
        block_control_t.Out++;
        if (block_control_t.Out > block_control_t.End)
            block_control_t.Out = block_control_t.Block;
    }
}

/****************************************** 串口中断发送实现参考 *********************************************/
// volatile uint8_t tx_buffer[256];
// volatile uint16_t tx_index;
// volatile uint16_t tx_length;
// volatile uint8_t tx_complete = 1;    // 1 表示空闲，0 表示发送中

/* 启动发送：复制数据，使能 TXE 中断 */
// void usart_send(uint8_t *data, uint16_t len) {
//     while(!tx_complete);              // 等待上次发送完成
//     tx_index = 0;
//     tx_length = len;
//     tx_complete = 0;
//     // 复制数据到发送缓冲区
//     for(uint16_t i = 0; i < len; i++) {
//         tx_buffer[i] = data[i];
//     }
//     // 使能 TXE（发送数据寄存器空）中断
//     usart_interrupt_enable(USART0, USART_INT_TBE);
//     // 写入第一个字节，触发 TXE 中断
//     usart_data_transmit(USART0, tx_buffer[tx_index++]);
// }

// void USART0_IRQHandler(void) {
//     /* 处理发送缓冲区空中断（TXE） */
//     if(usart_interrupt_flag_get(USART0, USART_INT_TBE) != RESET) {
//         if(tx_index < tx_length) {
//             // 还有数据：发送下一个字节
//             usart_data_transmit(USART0, tx_buffer[tx_index++]);
//         } else {
//             // 所有数据已从数据寄存器发出，关闭 TXE 中断，开启 TC 中断
//             usart_interrupt_disable(USART0, USART_INT_TBE);
//             usart_interrupt_enable(USART0, USART_INT_TC);
//         }
//     }

//     /* 处理发送完成中断（TC） */
//     if(usart_interrupt_flag_get(USART0, USART_INT_TC) != RESET) {
//         // 最后一个字节已从引脚完全发送
//         usart_interrupt_disable(USART0, USART_INT_TC);
//         tx_complete = 1;                // 通知主程序发送完成
//         // 若有需要，可在此处加回调或置标志
//     }
// }

/********************************************************************************************************/
