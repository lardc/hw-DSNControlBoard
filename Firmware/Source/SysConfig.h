#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

#include "stdinc.h"


// Flash loader options
#define BOOT_LOADER_VARIABLE			(*((volatile uint32_t *)0x20000000))
#define BOOT_LOADER_REQUEST				0x12345678
#define BOOT_LOADER_MAIN_PR_ADDR		0x08008000
//-----------------------------------------------

// System clock
#define SYSCLK							70000000	// Тактовая частота системной шины процессора
#define QUARTZ_FREQUENCY				20000000	// Частота кварца
// ----------------------------------------------

// USART
#define USART_BAUDRATE					115200		// Скорость USART
#define USART_FIFOlen					32			// Длина FIFO USART
// ----------------------------------------------

// Timers
#define TIMER7_uS						1000		// мкс
#define TIMER2_TICK_HZ					1000000		//
// ----------------------------------------------

// CAN
#define CAN_BAUDRATE					1000000		// Битрейт CAN
// ----------------------------------------------

// ADC2: диагностика силовых линий (PB2 = ADC2_IN12)
#define ADC_DIAG_CHANNEL				12
// ----------------------------------------------

// SPI3: SR и ЦАП IGBTGateDriver, ~1 МГц, MSB first, rising edge
#define SPI3_BAUDRATE_BITS				0x4			// APB1≈35 МГц / 32 ≈ 1.1 МГц
#define SPI3_LSB_FIRST					false
// ----------------------------------------------

#endif // __SYSCONFIG_H
