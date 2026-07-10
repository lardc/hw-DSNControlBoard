#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f30x.h"

#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwNCAN.h"
#include "ZwTIM.h"
#include "ZwDAC.h"
#include "ZwDMA.h"
#include "ZwADC.h"
#include "ZwEXTI.h"
#include "ZwSCI.h"
#include "ZwIWDG.h"
#include "ZwNFLASH.h"
#include "ZwSPI.h"

// Определения для выходных портов
GPIO_PortPinSettingMacro GPIO_LED				= {GPIOB, Pin_1};	// Мигающий зелёный индикатор на плате
GPIO_PortPinSettingMacro GPIO_FP_LED			= {GPIOA, Pin_3};	// Индикатор на передней панели
GPIO_PortPinSettingMacro GPIO_SD_EN				= {GPIOB, Pin_9};	// Формирование напряжения самодиагностики
//
// CS сдвиговых регистров на трёх ЭМ (аппаратный SPI1)
GPIO_PortPinSettingMacro GPIO_SPI_SS1			= {GPIOA, Pin_15};	// InputRelayBoard
GPIO_PortPinSettingMacro GPIO_SPI_SS2			= {GPIOB, Pin_6};	// OutThermRelayBoard
GPIO_PortPinSettingMacro GPIO_SPI_SS3			= {GPIOB, Pin_7};	// IORelayBoard (бывшие OutHVRelayBoard-1/2)
//
// Управление аппаратным контуром безопасности
GPIO_PortPinSettingMacro GPIO_SFT_ENABLE		= {GPIOB, Pin_5};	// OpenDrain, связан напрямую с OE сдвиговых регистров

// Определения для входных портов
GPIO_PortPinSettingMacro GPIO_SFT_IN			= {GPIOB, Pin_12};	// Вход с контура безопасности: 0 — норма, 1 — срабатывание
GPIO_PortPinSettingMacro GPIO_SPI_TEST_INPUT	= {GPIOB, Pin_4};

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_UART1_TX		= {GPIOA, Pin_9};
GPIO_PortPinSettingMacro GPIO_ALT_UART1_RX		= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_TX		= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_RX		= {GPIOA, Pin_11};
//
GPIO_PortPinSettingMacro GPIO_ALT_SPI1_SCLK		= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_ALT_SPI1_MOSI		= {GPIOA, Pin_7};

#endif // __BOARD_H
