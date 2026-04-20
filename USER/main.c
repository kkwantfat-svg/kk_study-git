#include "gd32f10x.h"                   // Device header
#include "systick.h"                    // Systick header
#include "gd32f10x_exti.h"              // EXTI header
#include "usart.h"
#include "timer.h"
#include "gd32f10x_timer.h"

/* DMA buffer */
uint8_t buff[256];

int main(void)
{
	/* USART0 init */
	USART0_init(115200);

	/* SysTick init */
	systick_config();
	/* DMA init */
	DMA0_Init();

	/* Timer init */
	Timer_init();

	while(1)
	{
		/* Print counter */
		u0_printf("*** count number: %d ***\r\n", timer_counter_read(TIMER1));
		/* Optional delay */
		//delay_1ms(100);
		/* Update pulse */
		set_output_pulse_value();
	}
}



