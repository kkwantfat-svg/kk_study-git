/*!
    \file    systick.c
    \brief   the systick configuration file

    \version 2026-02-12, V2.7.0, firmware for GD32F10x
*/

/*
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f10x.h"
#include "systick.h"

volatile static uint32_t delay;  // 全局变量，systick中断服务函数中会对它进行递减操作，delay_1ms函数会设置它的值，主程序中可以通过轮询该变量的值来实现延时功能
volatile static uint32_t current_time_ms = 0;   // 全局变量，systick中断服务函数中会对它进行递增操作，用于获取当前时刻的时间

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)   // 使能中断，配置中断优先级，但是没有进行优先级分组，默认使用NVIC的默认分组（NVIC_PRIGROUP_PRE0_SUB4，0 位抢占优先级，4 位子优先级），systick中断优先级设置为最高，确保systick中断能够及时响应，保证延时函数的准确性
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){  // 计算出每毫秒需要的时钟周期数为SystemCoreClock / 1000U，并将其作为参数传递给SysTick_Config函数
        while (1){
        }
    }  // SysTick_Config函数会根据传入的参数计算出合适的重载值，并配置systick定时器以产生周期性的中断，参数SystemCoreClock / 1000U表示每1毫秒产生一次中断，确保systick中断能够及时响应，保证延时函数的准确性
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);  // 设置systick中断优先级为最高，确保systick中断能够及时响应，保证延时函数的准确性，裸机中0是最高优先级，数值越大优先级越低
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    delay = count;

    while(0U != delay){
    }
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)  // 该函数会被systick中断服务函数调用，用于实现delay_1ms函数的延时功能，delay_1ms函数会设置全局变量delay的值，systick中断服务函数每次被调用时都会对delay进行递减操作，主程序中可以通过轮询该变量的值来实现延时功能
{
    if (0U != delay){
        delay--;
    }
}

// 当前tick数值自增
void current_time(void)
{
    current_time_ms ++;
}

// 获取当前tick值 单位为ms
uint32_t get_curent_time_ms(void)
{
    return current_time_ms;
}