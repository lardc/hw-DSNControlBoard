#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER						1	// Включение блока
#define ACT_DISABLE_POWER						2	// Выключение блока
#define ACT_CLR_FAULT							3	// Очистка всех fault
#define ACT_CLR_WARNING							4	// Очистка всех warning

#define ACT_DBG_IND								21	// Импульс индикатора передней панели
#define ACT_DBG_SFT_IN							22	// Состояние GPIO_SFT_IN → REG_DBG
#define ACT_DBG_FAN								23	// Управление FAN (REG_DBG: 0/1)
#define ACT_DBG_GATE_CTRL						24	// Импульс GateCtrl (REG_DBG: 0=H, 1=L)
#define ACT_DBG_SPI_MUX							25	// SPI_SCK_MUX (REG_DBG: 0=SR, 1=DAC)
#define ACT_DBG_DRV_CONFIG						26	// Конфиг драйвера из оперативных REG_GATE_* / REG_TEST_*
#define ACT_DBG_ONEWIRE_DISCOVER				125	// Поиск меток/DS18B20; REG_DBG: T+SW*10+GD*100+Probe*1000
#define ACT_DBG_READ_TEMP						126	// Температура адаптера °C → REG_DBG
#define ACT_DBG_ADAPTER_LIMITS					127	// MaxVoltage*1000 + MaxCurrent → REG_DBG

#define ACT_SET_ACTIVE							100	// Активация контура безопасности
#define ACT_SET_INACTIVE						101	// Деактивация контура безопасности

#define ACT_CONFIGURE							110	// Конфигурация формирования (1Wire + DPN + драйверы)
#define ACT_START_PULSE							111	// Запуск двойного импульса
#define ACT_STOP_PULSE							112	// Остановка формирования
#define ACT_SELF_TEST							113	// Самодиагностика

#define ACT_SAVE_TO_ROM							200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM					201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT					202	// Сброс DataTable в состояние по умолчанию

#define ACT_BOOT_LOADER_REQUEST					320	// Перезапуск процессора с целью перепрограммирования

#define ACT_FLASH_DIAG_INIT_READ				331	// Инициализировать начало считывания отладочной информации
#define ACT_FLASH_CNT_INIT_READ					334	// Перемещение указателя в область счетчиков
#define ACT_FLASH_CNT_READ_SYMBOL				335	// Считывание символа в области счетчиков
#define ACT_SET_COUNTER							336	// Установка значения счетчика
#define ACT_SAVE_COUNTERS						337	// Сохранить счетчики в памяти
#define ACT_ERASE_COUNTERS						338	// Удалить счетчики из памяти
#define ACT_FLASH_COUNTER_TO_EP					339	// Чтение массива счетчиков из flash в EP
#define ACT_FLASH_DIAG_TO_EP					340	// Чтение отладочной информации из flash в EP
// -----------------------------

// Сохраняемые регистры (NV)
//
#define REG_SAFETY_ACTIVE						2	// Активация системы безопасности
#define REG_DPN_CAN_ID							3	// DPN CAN ID
#define REG_DPN_EMULATED						4	// DPN emulation
#define REG_DRVH_OPEN_K							5	// К отпирающего ЦАП IGBTGateDriverH
#define REG_DRVH_CLOSE_K						6	// К запирающего ЦАП IGBTGateDriverH
#define REG_DRVL_OPEN_K							7	// К отпирающего ЦАП IGBTGateDriverL
#define REG_DRVL_CLOSE_K						8	// К запирающего ЦАП IGBTGateDriverL
#define REG_TEMP_COOLING						9	// Порог включения FAN, °C
#define REG_ADC_DIAG_K							10	// К АЦП диагностики силовых линий
#define REG_ADC_DIAG_OFFSET						11	// Смещение АЦП диагностики
//
#define REG_CNT_NUMBER							104	// Номер счетчика, в который будет записано значение из регистра 105
#define REG_CNT_VALUE							105	// Значение, которое будет записано в счетчик

// Несохраняемые регистры чтения-записи
//
#define REG_TEST_VOLTAGE						128	// Рабочее напряжение, В
#define REG_TEST_CURRENT						129	// Рабочий ток, А
#define REG_TEST_POS							130	// Позиция: 0 = H, 1 = L
#define REG_TEST_SCHEME							131	// Код схемы: SD=17, HB=18, LC=19, HC=20, DD=21
#define REG_TEST_PULSE_PAUSE					132	// Пауза между импульсами, мкс
#define REG_TEST_SECOND_PULSE					133	// Длительность второго импульса, мкс
#define REG_GATE_OPEN_V							134	// Отпирающее напряжение затвора, В
#define REG_GATE_CLOSE_V						135	// Запирающее напряжение затвора, В
#define REG_GATE_RG_H							136	// Затворное сопротивление H, Ом
#define REG_GATE_RG_L							137	// Затворное сопротивление L, Ом
//
#define REG_DBG									150	// Отладочный регистр

// Регистры только чтение
//
#define REG_DEV_STATE							192	// Регистр состояния
#define REG_FAULT_REASON						193	// Регистр Fault
#define REG_DISABLE_REASON						194	// Регистр Disable
#define REG_WARNING								195	// Регистр Warning
#define REG_PROBLEM								196	// Регистр Problem
#define REG_OP_RESULT							197	// Регистр результата операции
#define REG_SELF_TEST_OP_RESULT					198	// Регистр результата самотестирования
#define REG_SUB_STATE							199	// Регистр вспомогательного состояния
//
#define REG_TEMPERATURE							200	// Температура адаптера, °C
#define REG_SELECTED_INDUCTANCE					201	// Индуктивность с DPN, мкГн
#define REG_SELECTED_PULSE_WIDTH				202	// Длительность 1-го импульса с DPN, мкс
#define REG_ADAPTER_MAX_VOLTAGE					203	// MaxV с метки Probe
#define REG_ADAPTER_MAX_CURRENT					204	// MaxI с метки Probe
#define REG_ADC_DIAG_RAW						205	// Сырое/приведённое значение АЦП диагностики
//
#define REG_EXT_UNIT_ERROR_CODE					220	// Ошибка интерфейса CAN: код ошибки
#define REG_EXT_UNIT_FUNCTION					221	// Ошибка интерфейса CAN: код функции
#define REG_EXT_UNIT_EXT_DATA					222	// Ошибка интерфейса CAN: расширенная информация
// -----------------------------
#define REG_FWINFO_SLAVE_NID					256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID					257	// Device CAN master node ID (if presented)
#define REG_FWINFO_STR_LEN						260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN					261	// Beginning of the information string record
#define REG_MEM_SYMBOL							299	// Считанный по адресу памяти символ

// DPN actions (коды команд на стороне DPNControlBoard)
#define ACT_DPN_ENABLE_POWER					1	// Включение блока DPN
#define ACT_DPN_DISABLE_POWER					2	// Выключение / принудительная остановка DPN
#define ACT_DPN_CLR_FAULT						3	// Очистка fault DPN
#define ACT_DPN_CLR_WARNING						4	// Очистка warning DPN
#define ACT_DPN_CONFIG							100	// Конфигурация узла по заданным параметрам
#define ACT_DPN_SAFETY_TRIG_CLEAR				101	// Сброс состояния SafetyTrig
#define ACT_DPN_SELF_TEST						110	// Запуск самотестирования DPN

// DPN registers (номера регистров на стороне DPN)
#define REG_DPN_VOLTAGE							128	// Напряжение испытания, В
#define REG_DPN_DUT_POSITION					129	// Позиция DUT
#define REG_DPN_CURRENT							130	// Ток испытания, А
#define REG_DPN_MAX_DURATION					131	// Максимальное время измерения, мкс
#define REG_DPN_DEV_STATE						192	// Регистр состояния
#define REG_DPN_FAULT_REASON					193	// Регистр Fault
#define REG_DPN_DISABLE_REASON					194	// Регистр Disable
#define REG_DPN_WARNING							195	// Регистр Warning
#define REG_DPN_PROBLEM							196	// Регистр Problem
#define REG_DPN_OP_RESULT						197	// Регистр результата операции
#define REG_DPN_INDUCTANCE						203	// Выбранная индуктивность
#define REG_DPN_MEASURE_DURATION				204	// Расчётное время измерения, мкс

// Operation results
#define OPRESULT_NONE							0	// Нет информации / не завершено
#define OPRESULT_OK								1	// Операция успешна
#define OPRESULT_FAIL							2	// Операция завершилась с ошибкой

// Fault and disable codes
#define DF_NONE									0	// Ошибок нет
#define DF_DPN_INTERFACE						2	// Проблема связи по интерфейсу с DPN
#define DF_DPN									3	// DPN в состоянии Fault
#define DF_DPN_FAILED_TO_FINISH					4	// DPN не успел выполнить команду / не перешёл в готовность
#define DF_SELF_TEST							5	// Ошибка самодиагностики
#define DF_SAFETY								6	// Срабатывание контура безопасности

// Problem
#define PROBLEM_NONE							0
#define PROBLEM_DPN_NOT_READY					2	// DPN не готов
#define PROBLEM_DPN_FAILED_COMMAND				4	// OPResult DPN = fail без fault
#define PROBLEM_OW_BUS							10	// Ошибка шины 1-Wire / поиск
#define PROBLEM_OW_INVENTORY					11	// Неполный набор меток адаптера
#define PROBLEM_OW_TEMP							12	// DS18B20 недоступен
#define PROBLEM_VALIDATE_SCHEME					13	// Неверное число SwitchBoard для схемы
#define PROBLEM_VALIDATE_VOLTAGE				14	// Напряжение выше MaxV метки
#define PROBLEM_VALIDATE_CURRENT				15	// Ток выше MaxI метки
#define PROBLEM_SELFTEST						20	// Отказ сценария самодиагностики

// Warning
#define WARNING_NONE							0	// Предупреждений нет

// User Errors
#define ERR_NONE								0
#define ERR_CONFIGURATION_LOCKED				1	// Устройство защищено от записи
#define ERR_OPERATION_BLOCKED					2	// Операция не может быть выполнена в текущем состоянии
#define ERR_DEVICE_NOT_READY					3	// Устройство не готово для смены состояния
#define ERR_WRONG_PWD							4	// Неправильный ключ

// Endpoints
#define EP_DiagData								20	// Диагностические данные из flash

#endif //  __DEV_OBJ_DIC_H
