#include "gd32f10x.h"                   // Device header
#include "gd32f10x_timer.h"
#include "gd32f10x_gpio.h"
#include "timer.h"
#include "usart.h"
#include "systick.h"                    // Systick header

uint8_t flag = 0;

void Timer_init(void)
{
    rcu_periph_clock_enable(RCU_TIMER1);   // init clock
    rcu_periph_clock_enable(RCU_GPIOA);   // init clock

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);    // pa0  复用推挽

    timer_parameter_struct timer_struct;
    
    /* deinit a TIMER */
    timer_deinit(TIMER1);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_struct);
    
    /* 内部时钟 0.1s */
    timer_struct.alignedmode = TIMER_COUNTER_EDGE;        // 边沿对齐模式
    timer_struct.clockdivision = TIMER_CKDIV_DIV1;        // 不分频
    timer_struct.counterdirection = TIMER_COUNTER_UP;     // 向上计数模式
    timer_struct.period = 1000 - 1;             // 自动重装载数值
    timer_struct.prescaler = 10800 - 1;           // 预分频数值
    
    /* initialize TIMER counter */
    timer_init(TIMER1, &timer_struct);

    // pwm 配置
    timer_oc_parameter_struct ocpara_struct;
    timer_channel_output_struct_para_init(&ocpara_struct);
    
    ocpara_struct.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
    ocpara_struct.ocpolarity = TIMER_OC_POLARITY_HIGH;  // CNT < CCR 时输出有效电平，CNT ≥ CCR 时输出无效电平”。但这里的“有效电平”到底是高还是低？就由 ocpolarity 决定
    ocpara_struct.outputstate = TIMER_CCX_ENABLE;       // 通道输出使能

    timer_channel_output_config(TIMER1, TIMER_CH_0, &ocpara_struct);
    
    timer_channel_output_mode_config(TIMER1, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, 500);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);   //关闭影子寄存器

    timer_enable(TIMER1);
}

void set_output_pulse_value(void)
{
    for(uint16_t i = 0; i < 1000; i++)
    {
        timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, i);
        delay_1ms(1);
    }
}

