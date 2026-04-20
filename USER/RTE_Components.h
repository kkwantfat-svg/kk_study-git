#ifndef GD32F10X_RTE_H
#define GD32F10X_RTE_H

/*  定义用到的外设所需宏定义，参见 gd32f10x_libopt.h  */

#define  RTE_DEVICE_STDPERIPHERALS_RCU       // 提供时钟使能、复位控制、系统时钟配置等功能，几乎所有外设都依赖它
#define  RTE_DEVICE_STDPERIPHERALS_GPIO      
#define  RTE_DEVICE_STDPERIPHERALS_MISC      // 中断优先级分组、中断使能/屏蔽等
#define  RTE_DEVICE_STDPERIPHERALS_DMA

#endif
