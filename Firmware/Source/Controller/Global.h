#ifndef __GLOBAL_H
#define __GLOBAL_H

// Definitions
// 
#define	SCCI_TIMEOUT_TICKS						1000	// Таймаут интерфейса SCCI, мс
#define	BCCIM_TIMEOUT_TICKS						1000	// Таймаут протоколоа мастер BCCI, мс
#define EP_WRITE_COUNT							0		// Количество массивов для записи
#define EP_COUNT								0		// Количество массивов для чтения
#define FEP_COUNT								1		// Количество массивов для чтения типа float
#define VALUES_x_SIZE							0		// Размер массивов
#define VALUES_DIAG_SIZE						64		// Размер массива диагностических данных
#define ENABLE_LOCKING							FALSE	// Защита NV регистров паролем

// Временные параметры
#define TIME_LED_BLINK							500		// Мигание светодиодом, мс
#define TIME_FP_LED_FAULT_BLINK					250		// Мигание светодиодом в состоянии Fault
#define TIME_FP_LED_ON_STATE					500		// Время зажигания индикатора на передний панели в нормальном режиме, мс
#define TIME_COOLING_PERIOD_MS					10000	// Период опроса температуры / FAN, мс
#define COUNTER_SAVE_TIMEOUT_MS					1800000	// Период автосохранения счётчиков во flash, мс
#define COMM_DELAY_MS							20		// Задержка коммутации реле, мс
#define TIME_SPI_DELAY_US						5		// Время задержки SPI

// Параметры измерения
#define ADC_REF_VOLTAGE							3300.0f	// Опорное напряжение АЦП, мВ
#define ADC_RESOLUTION							4095	// Разрешение АЦП
#define DAC_VREF								3000.0f	// Опорное напряжение ЦАП, мВ
#define DAC_RESOLUTION_16						65535.0f


#endif //  __GLOBAL_H
