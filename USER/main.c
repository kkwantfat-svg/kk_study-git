#include "gd32f10x.h"                   // Device header
#include "systick.h"                    // Systick header
#include "gd32f10x_exti.h"              // EXTI header
#include "usart.h"
#include "timer.h"
#include "gd32f10x_timer.h"

/* USART DMA发送缓冲区 */
uint8_t buff[256];

int main(void)
{
	/* 初始化USART0，波特率115200，用于串口打印调试信息 */
	USART0_init(115200);

	/* 配置系统滴答定时器，为延时等基础时基功能提供支持 */
	systick_config();
	/* 初始化DMA，用于串口等外设的数据搬运 */
	DMA0_Init();

	/* 初始化定时器相关功能，包括PWM/计数等配置 */
	Timer_init();

	while(1)
	{
		/* 读取TIMER1当前计数值，并通过串口输出 */
		u0_printf("*** count number: %d ***\r\n", timer_counter_read(TIMER1));
		/* 如需降低打印频率，可打开该延时 */
		//delay_1ms(100);
		/* 根据当前配置更新输出脉冲参数 */
		set_output_pulse_value();
	}
}



