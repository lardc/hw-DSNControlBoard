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

//
// PA1: в ТТ указан и как вход safety, и как TIM2_CH2 (SelfTestResult).
// Принято: AF TIM2_CH2; уровень safety читается с того же пина (IDR) до/вне capture.
GPIO_PortPinSettingMacro GPIO_SFT_IN			= {GPIOA, Pin_1};
GPIO_PortPinSettingMacro GPIO_SFT_IGNORE		= {GPIOA, Pin_2};	// Игнорирование входа безопасности
GPIO_PortPinSettingMacro GPIO_ST_DISCHARGE		= {GPIOA, Pin_3};	// Разряд конденсатора диагностики
GPIO_PortPinSettingMacro GPIO_ST_START			= {GPIOA, Pin_4};	// Запуск диагностики затворной цепи
//
GPIO_PortPinSettingMacro GPIO_DQ_PWR			= {GPIOA, Pin_5};
GPIO_PortPinSettingMacro GPIO_DQ_IN				= {GPIOA, Pin_6};
GPIO_PortPinSettingMacro GPIO_DQ_CTRL			= {GPIOA, Pin_7};
//
GPIO_PortPinSettingMacro GPIO_RCK_EXP			= {GPIOA, Pin_8};	// Latch SR расширения портов (OD)
GPIO_PortPinSettingMacro GPIO_OE_EXP			= {GPIOA, Pin_9};	// Инвертированный OE SR расширения (OD)
GPIO_PortPinSettingMacro GPIO_GATE_CTRL_H		= {GPIOA, Pin_10};	// Импульс формирования IGBTGateDriverH (OD)
GPIO_PortPinSettingMacro GPIO_GATE_CTRL_L		= {GPIOA, Pin_11};	// Импульс формирования IGBTGateDriverL (OD)
//
GPIO_PortPinSettingMacro GPIO_SPI_SCK_MUX		= {GPIOB, Pin_4};	// 0: SCK→SR драйверов; 1: SCK→DAC (OD)
GPIO_PortPinSettingMacro GPIO_RCK_H				= {GPIOB, Pin_6};	// Latch SR IGBTGateDriverH (OD)
GPIO_PortPinSettingMacro GPIO_RCK_L				= {GPIOB, Pin_7};	// Latch SR IGBTGateDriverL (OD)
//
GPIO_PortPinSettingMacro GPIO_FAN				= {GPIOB, Pin_12};
GPIO_PortPinSettingMacro GPIO_SCOPE_SYNC		= {GPIOB, Pin_13};
GPIO_PortPinSettingMacro GPIO_LED				= {GPIOB, Pin_14};
GPIO_PortPinSettingMacro GPIO_FP_LED			= {GPIOB, Pin_15};

// Аналоговый вход диагностики силовых линий
GPIO_PortPinSettingMacro GPIO_ADC_DIAG			= {GPIOB, Pin_2};	// ADC2 IN12

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_UART3_TX		= {GPIOB, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_UART3_RX		= {GPIOB, Pin_11};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_RX		= {GPIOB, Pin_8};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_TX		= {GPIOB, Pin_9};
GPIO_PortPinSettingMacro GPIO_ALT_SPI3_SCK		= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_ALT_SPI3_MOSI		= {GPIOB, Pin_5};
GPIO_PortPinSettingMacro GPIO_ALT_TIM2_CH2		= {GPIOA, Pin_1};	// SelfTestResult capture

#endif // __BOARD_H
