#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER						1	// Включение блока
#define ACT_DISABLE_POWER						2	// Выключение блока
#define ACT_CLR_FAULT							3	// Очистка всех fault
#define ACT_CLR_WARNING							4	// Очистка всех warning

#define ACT_SET_ACTIVE							100	// Команда активации контура безопасности
#define ACT_SET_INACTIVE						101	// Команда деактивации контура безопасности

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
#define REG_SAFETY_ACTIVE						2	// Активация системы безопасности
#define REG_DPN_CAN_ID							3	// DPN CAN ID
#define REG_DPN_EMULATED						4	// DPN emulation
#define REG_DRVH_OPEN_K							5	// К отпирающего ЦАП IGBTGateDriverH
#define REG_DRVH_CLOSE_K						6	// К запирающего ЦАП IGBTGateDriverH
#define REG_DRVL_OPEN_K							7	// К отпирающего ЦАП IGBTGateDriverL
#define REG_DRVL_CLOSE_K						8	// К запирающего ЦАП IGBTGateDriverL
//
#define REG_CNT_NUMBER							104	// Номер счетчика, в который будет записано значение из регистра 105
#define REG_CNT_VALUE							105	// Значение, которое будет записано в счетчик

// Несохраняемые регистры чтения-записи
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

#define REG_EXT_UNIT_ERROR_CODE					220	// Ошибка интерфейса CAN: код ошибки
#define REG_EXT_UNIT_FUNCTION					221	// Ошибка интерфейса CAN: код функции
#define REG_EXT_UNIT_EXT_DATA					222	// Ошибка интерфейса CAN: расширенная информация

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
#define DF_DPN_INTERFACE						2	// Проблема связи по интерфейсу с DPN
#define DF_DPN									3	// DPN в состоянии Fault
#define DF_DPN_FAILED_TO_FINISH					4	// DPN не смог/успел выполнить команду и не перешёл в готовность

// Problem
#define PROBLEM_NONE							0
#define PROBLEM_DPN_NOT_READY					2	// DPN не готов
#define PROBLEM_DPN_FAILED_COMMAND				4	// OPResult DPN выставился fail без фолта

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

// DPN actions (коды команд на стороне DPN)
//
#define ACT_DPN_ENABLE_POWER					1	// Включение блока
#define ACT_DPN_DISABLE_POWER					2	// Выключение блока
#define ACT_DPN_CLR_FAULT						3	// Очистка fault
#define ACT_DPN_CLR_WARNING						4	// Очистка warning
//
#define ACT_DPN_CONFIG							100	// Конфигурация узла по заданным параметрам
#define ACT_DPN_SAFETY_TRIG_CLEAR				101	// Сброс состояния SafetyTrig
//
#define ACT_DPN_SELF_TEST						110	// Запуск самотестирования DPN
//
// DPN registers
//
#define REG_DPN_VOLTAGE							128	// Напряжение испытания, В
#define REG_DPN_DUT_POSITION					129	// Позиция DUT
#define REG_DPN_CURRENT							130	// Ток испытания, А
#define REG_DPN_MAX_DURATION					131	// Максимальное время измерения, мкс
//
#define REG_DPN_DEV_STATE						192	// Регистр состояния
#define REG_DPN_FAULT_REASON					193	// Регистр Fault
#define REG_DPN_DISABLE_REASON					194	// Регистр Disable
#define REG_DPN_WARNING							195	// Регистр Warning
#define REG_DPN_PROBLEM							196	// Регистр Problem
#define REG_DPN_OP_RESULT						197	// Регистр результата операции

#endif //  __DEV_OBJ_DIC_H
