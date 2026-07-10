#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER						1	// Включение блока
#define ACT_DISABLE_POWER						2	// Выключение блока
#define ACT_CLR_FAULT							3	// Очистка всех fault
#define ACT_CLR_WARNING							4	// Очистка всех warning
//
#define ACT_DBG_FP_LED							21	// Проверка работы индикатора на передней панели
// 22 и 23 — зарезервированы: бывшие ACT_DBG_SF_RED_LED/ACT_DBG_SF_GRN_LED удаляются (светодиоды безопасности убраны из схемы MXU303)
#define ACT_DBG_WRITE_SPI						24	// Проверка работы коммутации тестовой комбинации
#define ACT_DBG_SPI_RST							25	// Сброс сдвиговых регистров
#define ACT_DBG_SELF_TEST_MEASURE				26	// Измерение значения напряжение системы самодиагностики
#define ACT_DBG_SD_EN							27	// Проверка формирования напряжения самодиагностики
#define ACT_DBG_SFT_ENABLE						28	// Импульс SFT_ENABLE (перевод OE сдвиговых регистров в high-Z)
#define ACT_DBG_SFT_IN							29	// Прочитать состояние входа SFT_IN в REG_DBG
#define ACT_DBG_SPI_SS							30 	// Проверка SPI_SS

#define ACT_SET_ACTIVE							100	// Команда активации контура безопасности
#define ACT_SET_INACTIVE						101	// Команда деактивации контура безопасности

#define ACT_COMM_NONE							110 // Отключение всех реле
#define ACT_COMM_IGES_POS_PULSE					111 // Режим измерения тока утечки затвора-эмиттера (положительный импульс)
#define ACT_COMM_IGES_NEG_PULSE					112 // Режим измерения тока утечки затвора-эмиттера (отрицательный импульс)
#define ACT_COMM_UGE_TH							113 // Режим измерения порогового напряжения затвор-эмиттер
// 114 — зарезервирован: бывший ACT_COMM_QG удалён (измерение QG выведено из MXU303)
#define ACT_COMM_UCE_SAT						115 // Режим измерения напряжения насыщения коллектор-эмиттер
#define ACT_COMM_UFW_CHOPPER_DIODE				116 // Режим измерения прямого напряжения обратно-параллельного диода и диода чоппера
#define ACT_COMM_ICES_OR_IRRM					117 // Режим измерения ICES (IGBT) / IRRM (диод)
#define ACT_COMM_THERMISTOR						118 // Режим измерения сопротивления термистора
#define ACT_COMM_NO_PE							119 // Отключения защитного заземления

#define ACT_SELFT_TEST							120	// Запуск режима самотестирования

#define ACT_SAVE_TO_ROM							200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM					201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT					202	// Сброс DataTable в состояние по умолчанию

#define ACT_BOOT_LOADER_REQUEST					320	// Перезапуск процессора с целью перепрограммирования

#define ACT_FLASH_DIAG_INIT_READ				331 // Инициализировать начало считывания отладочной информации
#define ACT_FLASH_CNT_INIT_READ					334	// Перемещение указателя в область счетчиков
#define ACT_FLASH_CNT_READ_SYMBOL				335	// Считывания символа в области счетчиков
#define ACT_SET_COUNTER							336	// Установка значения счетчика
#define ACT_SAVE_COUNTERS						337	//
#define ACT_ERASE_COUNTERS						338	//
#define ACT_FLASH_COUNTER_TO_EP					339 // Выполнить чтение массива из памяти счетчиков отработки в EP
#define ACT_FLASH_DIAG_TO_EP					340 // Выполнить чтение массива из памяти отладочной информации в EP
// -----------------------------

// Сохраняемые регистры
//
#define REG_SFTST_CLOSED_MAX_VOLTAGE			0	// Максимальное напряжение на замкнутом реле, мВ
#define REG_SAFETY_DELAY						1	// Задержка отключения коммутации, мс
#define REG_SAFETY_ACTIVE						2	// Активация системы безопасности
#define REG_PMXU_CAN_ID							3	// PMXU CAN ID
#define REG_PMXU_EMULATED						4	// PMXU emulation
#define REG_SFTST_OPENED_MIN_VOLTAGE			5	// Минимальное напряжение на разомкнутом реле, мВ
//
#define REG_MODULE_TYPE							70	// Module type(Заглушка)
#define REG_DUT_POSITION						71	// Регистр выбора позции тестируемого прибора (1 или 2)
#define REG_DUT_CASE							72	// Тип корпуса тестируемого прибора
#define REG_DUT_SCHEME							73	// Схема подключения внутри корпуса
//
#define REG_EN_SFTY_IN1							80	// Enable safety input #1(Заглушка)
#define REG_EN_SFTY_IN2							81	// Enable safety input #2(Заглушка)
#define REG_EN_SFTY_IN3							82	// Enable safety input #3(Заглушка)
#define REG_EN_SFTY_IN4							83	// Enable safety input #4(Заглушка)
//
#define REG_DEV_STATE							96	// Регистр состояния
#define REG_FAULT_REASON						97	// Регистр Fault
#define REG_DISABLE_REASON						98	// Регистр Disable
#define REG_WARNING								99	// Регистр Warning
#define REG_PROBLEM								100	// Регистр Problem
#define REG_OP_RESULT							101	// Регистр результата операции
#define REG_SELF_TEST_OP_RESULT					102	// Регистр результата самотестирования
#define REG_SUB_STATE							103	// Регистр вспомогательного состояния

#define REG_CNT_NUMBER							104	// Номер счетчика, в который будет записано значение из регистра 105
#define REG_CNT_VALUE							105	// Значение, которое будет записано в счетчик

// Несохраняемые регистры чтения-записи
//
#define REG_DBG									150	// Отладочный регистр
#define REG_DBG2								151	// Отладочный регистр
//
// Регистры только чтение
#define REG_SELF_TEST_FAILED_STATE				200	// Указывает состояние реле или цепи, на котором произошёл сбой (замкнуто / разомкнуто)
#define REG_SELF_TEST_FAILED_RELAY				201	// Номер реле, на котором обнаружен отказ

#define REG_EXT_UNIT_ERROR_CODE					220	// Ошибка интерфейса CAN: код ошибки
#define REG_EXT_UNIT_FUNCTION					221	// Ошибка интерфейса CAN: код функции
#define REG_EXT_UNIT_EXT_DATA					222	// Ошибка интерфейса CAN: расширенная информация

// Информация о последней коммутации (для диагностики повторных/некорректных вызовов)
#define REG_LAST_CMD							252	// Код последней команды коммутации (ACT_COMM_*)
#define REG_LAST_POS							253	// Позиция DUT на момент последней коммутации
#define REG_LAST_CASE							254	// Тип корпуса DUT на момент последней коммутации
#define REG_LAST_TYPE							255	// Схема DUT на момент последней коммутации (REG_DUT_SCHEME)
// -----------------------------
#define REG_FWINFO_SLAVE_NID					256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID					257	// Device CAN master node ID (if presented)

#define REG_FWINFO_STR_LEN						260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN					261	// Begining of the information string record

#define REG_MEM_SYMBOL							299	// Считанный по адресу памяти символ


// Operation results
#define OPRESULT_NONE							0	// No information or not finished
#define OPRESULT_OK								1	// Operation was successful
#define OPRESULT_FAIL							2	// Operation failed

//  Fault and disable codes
#define DF_NONE									0
#define DF_SELF_TEST							1	// Проблема реле
#define DF_PMXU_INTERFACE						2	// Проблема связи по интерфейсу с PMXU
#define DF_PMXU									3	// PMXU в состоянии Fault
#define DF_PMXU_FAILED_TO_FINISH				4	// PMXU не смог\успел выполнить коммутацию и не перешел в состояние готовности

// Problem
#define PROBLEM_NONE							0
#define PROBLEM_INCORRECT_DUT					1	// Выбранная команда не может быть выполнена над текущим DUT
#define PROBLEM_PMXU_NOT_READY					2	// PMXU не готов
#define PROBLEM_INCORRECT_COMMAND				3 	// Вызвана команда не обрабатываемая в валидации
#define PROBLEM_PMXU_FAILED_COMMAND				4	// OPResult PMXU выставился fail без фолта

//  Warning
#define WARNING_NONE							0

//  User Errors
#define ERR_NONE								0
#define ERR_CONFIGURATION_LOCKED				1	//  Устройство защищено от записи
#define ERR_OPERATION_BLOCKED					2	//  Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY					3	//  Устройство не готово для смены состояния
#define ERR_WRONG_PWD							4	//  Неправильный ключ

// Endpoints

#define EP_DiagData								20	// External information from flash

// PMXU actions
//
#define ACT_PMXU_ENABLE_POWER					1	// Включение блока
#define ACT_PMXU_DISABLE_POWER					2	// Выключение блока
#define ACT_PMXU_CLR_FAULT						3	// Очистка всех fault
#define ACT_PMXU_CLR_WARNING					4	// Очистка всех warning
//
#define ACT_PMXU_SET_ACTIVE						100	// Команда активации контура безопасности
#define ACT_PMXU_SET_INACTIVE					101	// Команда деактивации контура безопасности
//
#define ACT_PMXU_SELFTEST						120	// Команда завпуска SelfTest
//
#define ACT_PMXU_COMM_PE						110 // Отключение всех реле, замыкание шин на PE
#define ACT_PMXU_COMM_ICES						111 // Режим измерения тока утечки коллектор-эмиттер (ICES/IRRM)
#define ACT_PMXU_COMM_VCESAT					112 // Режим измерения напряжения насыщения коллектор-эмиттер IGBT транзисторов
#define ACT_PMXU_COMM_VF						113 // Режим измерения постоянного прямого напряжения диода чоппера и обратно-параллельного диода
// 114 — зарезервирован: бывший ACT_PMXU_COMM_QG удалён (команда ACT_COMM_QG выведена из MXU303)
#define ACT_PMXU_COMM_NO_PE						115 // Отключение всех реле
//
// PMXU registers
//
#define REG_PMXU_DUT_POSITION					128	// Регистр выбора позции тестируемого прибора (1 или 2)
#define REG_PMXU_DEV_CASE						129	// Тип корпуса прибора

#define REG_PMXU_DEV_STATE						192	// Регистр состояния
#define REG_PMXU_FAULT_REASON					193	// Регистр Fault
#define REG_PMXU_DISABLE_REASON					194	// Регистр Disable
#define REG_PMXU_PROBLEM						196	// Регистр Problem
#define REG_PMXU_OP_RESULT						197	// Регистр результата операции
//
// PMXU faults
//
#define DF_PMXU_NONE							0
#define DF_PMXU_SELF_TEST						1	// Проблема с реле/контактором во время самотестирования
#define DF_PMXU_CONTACTOR_FAULT					2	// Проблема с контактором, маска неисправных в REG_FAILED_CONTACTOR
#define DF_PMXU_LOW_PRESSURE					3	// Низкое давление в пневмосистеме

// PMXU Problems
#define PROBLEM_PMXU_NONE						0
#define PROBLEM_PMXU_INCORRECT_DUT				1	// Выбранная команда не может быть выполнена над текущим DUT

#endif //  __DEV_OBJ_DIC_H
