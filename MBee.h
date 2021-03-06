﻿/* "Системы модули и компоненты" ("СМК"). 2020. Москва.
Библиотека MBee-Arduino.
Распространяется свободно. Надеемся, что программные продукты, созданные
с помощью данной библиотеки будут полезными, однако никакие гарантии, явные или
подразумеваемые не предоставляются.

The MIT License(MIT)

MBee-Arduino Library.
Copyright © 2020 Systems, modules and components. Moscow. Russia.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files(the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Code adapted from  XBee-Arduino library XBee.h. Copyright info below.
* @file       XBee.h
* @author     Andrew Rapp
* @license    This project is released under the GNU License
* @copyright  Copyright (c) 2009 Andrew Rapp. All rights reserved
* @date       2009
* @brief      Interface to the wireless XBee modules
*/

#ifndef MBee_h
#define MBee_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <inttypes.h>

//01.01.2011 0:00:00 //Начало эпохи для всех проектов "СМК".
#define SYSMC_BASE_YEAR 2011
#define SYSMC_BASE_DAY_OF_WEEK 6 //Тогда была суббота...

//Кодирование рабочего диапазона.
#define FREQ_2400     0x00
#define FREQ_868      0x40
#define FREQ_433      0x80
#define FREQ_868_2400 0xC0

//Кодирование вариантов модулей.
#define CC2530F256_CC2591     0x20
#define CC2530F256_NO_AMP     0x30
#define CC430F5137_NO_AMP     0x00
#define CC430F5137_CC1190     0x0C
#define CC430F5137_RF5838     0x1C 
#define CC1310_NO_AMP         0x24
#define CC1350_NO_AMP         0x28

//Кодирование ревизий модулей.
#define REV_0   0x00
#define REV_1   0x01
#define REV_2   0x02
#define REV_3   0x03

//Идентификаторы модулей.
#define MODULE_MBee3_0                  (FREQ_2400|CC2530F256_NO_AMP|REV_0)
#define MODULE_MBee3_1                  (FREQ_2400|CC2530F256_NO_AMP|REV_1)
#define MODULE_MBee2_1                  (FREQ_2400|CC2530F256_CC2591|REV_1)
#define MODULE_MBee_S1G_3_0_CC430_ext   (FREQ_868|CC430F5137_NO_AMP|REV_0)
#define MODULE_EM_CC430F5137_900        (FREQ_868|CC430F5137_NO_AMP|REV_1)
#define MODULE_MBee_S1G_3_0_CC1110_ext  (FREQ_868|CC1110F32_NO_AMP|REV_0)  
#define MODULE_MBee_S1G_2_0_CC1110_ext  (FREQ_868|CC1110F32_CC1190|REV_0) 
#define MODULE_MBee_S1G_2_0_CC430_ext   (FREQ_868|CC430F5137_CC1190|REV_0)
#define MODULE_MBP_SIG_1_0_CC430_ext    (FREQ_868|CC430F5137_RF5838|REV_0)
#define MODULE_MBee_DUAL_3_3_CC1310     (FREQ_868|CC1310_NO_AMP|REV_3)
#define MODULE_MBee_DUAL_3_3_CC1350     (FREQ_868_2400|CC1350_NO_AMP|REV_3)

//Компилятор C++11 имеет возможность оптимизации при использовании ключевого слова constexpr за счет исключения инициализационного кода для глобальных объектов. Для
//поддержки более старых версий компилятором это ключевое слово исключается.
#if __cplusplus >= 201103L
  #define CONSTEXPR constexpr
#else
  #define CONSTEXPR
#endif

//Константа, возвращаемая методами классов, использующие обратный вызов (сallbacks-функции), если заданный тип API-фрейма не принят за фиксированное время.
#define MBEE_WAIT_TIMEOUT 0xFF

/**
Основной интерфейс для связи с радиомодулями MBee всех серий.
Содержит функции для отправки и приема пакетов по радио через UART интерфейс.
Необходимо, чтобы модуль находился в пакетном режиме UART как на прием, так и на передачу.
<p/>
Код исполняется микроконтроллером в одном потоке, поэтому необходимо, чтобы чтение данных из буфера 
выполнялась бы со скоростью, превыщающей их поступление от модуля. В противном случае
будет иметь место потеря данных. 
Поскольку Ардуино имеет длину приемного буфера всего 128 байт, его могут переполнить всего 2 
эфирных пакета. Во избежание этого пользователь библиотеки должен самостоятельно определить период
вызова функций, выполняющих чтение приемного буфера UART таких как readPacket(...).
<p/>
С целью экономии ресурсов микроконтроллера Arduino, библиотека поддерживает хранение только одного 
принятого пакета в каждый момент времени. Это означает, то скетч должен полностью обработать предыдущий
пакет, прежде чем вызывать очередную функцию чтения readPacket(...).
<p/>
Размер памяти для хранения принятого пакета определяется константой MAX_FRAME_DATA_SIZE, которая зависит
от программного обеспечения, загруженного в радиомодуль. Эта константа соответствует максимально 
возможной длине пакета от модуля. Если в Вашем проекте используются пакеты с длиной меньше максимальной,
то Вы можете точно настроить значение константы MAX_FRAME_DATA_SIZE с целью экономии ресурсов
микроконтроллера. 
\author </i> von Boduen. Special thanx to Andrew Rapp.
*/
class MBee
{
public:
 /**
 Чтение всех байт, имеющихся в приемном буфере UART. Выполняется до тех пор, пока пакет не будет разобран полностью,
 не опустошится буфер или не произойдет какая-либо ошибка.
 Вы можете вызывать <i>MBee</i>.getResponse().isAvailable() после вызова этой функции, для того, чтобы определить
 был ли принят пакет. Для получения информации об ошибках при приеме пакета пользуйтесь методом <i>MBee</i>.getResponse().isError().
 <p/>
 Функция работает быстро, потому что не ждет получения данных по UART, а работает с байтами, имеющимися в буфере на
 момент вызова.
 <p/>
 ВНИМАНИЕ! Вызов этой функции очищает буфер принятого сообщения, следовательно перед ее вызовом предыдущее сообщение должно быть
 полностью обработано.
 Метод является чисто виртуальным (pure virtual), поэтому используется присваивание = 0;
 */
 virtual void readPacket() = 0;
 
 /**
 Ожидает приема пакета от модуля в течение <i>timeout</i> миллисекунд. Возвращает true, если пакет принят и false, если заданный
 интервал истек.
 Метод является чисто виртуальным (pure virtual), поэтому используется присваивание = 0;
 */ 
 virtual bool readPacket(int timeout) = 0;
 
 /**
 Ожидает приема пакета до тех пор, пока он не будет получен или не произойдет какая-либо ошибка.
 ВНИМАНИЕ! Использовать эту функцию следует осторожно, поскольку Ардуино может зависнуть, если пакет (с ошибками или без) так и не будет принят.
 Метод является чисто виртуальным (pure virtual), поэтому используется присваивание = 0;
 */
 virtual void readPacketUntilAvailable() = 0;
 
 /** 
 Передача данных модулю или удаленному узлу. Формат пакета зависит от типа объекта (программного обеспечения модуля). См. справку по поддерживаемым
 методам для конкретного проекта (SerialStar, MB-ZigBee и т.д).
 Метод является чисто виртуальным (pure virtual), поэтому используется присваивание = 0;
 */ 
 virtual void send(void*) = 0;

 /**
 Инициализирует соединение с радиомодулем на заданном COM-порте.
 */ 
 void begin(Stream &serial); 
 
 /**
 Назначает последовательный порт. Работает только на платах с несколькими аппаратными UART (Mega и т.п.)
 */
 void setSerial(Stream &serial);
 
 /**
 Проверяет наличие принятых данных в буфере UART.
 */ 
 bool available();
 
 /**
 Считывает 1 байт из входного буфера UART.
 */ 
 uint8_t read();
 
 /**
 Передает 1 байт по UART.
 */
 void write(uint8_t val);
 Stream* _serial;
};

/**
Базовый класс для работы с модулями диапазона 868 МГц.
*/
class MBee868 : public MBee
{
};

/**
Базовый класс для работы с модулями диапазона 2,4 ГГц. Находится в состоянии разработки.
*/
class MBee2400 : public MBee
{
};

/**
Класс для представления адреса модема.
*/
class MBeeAddress 
{
public:
 CONSTEXPR MBeeAddress()
 {
  
 };
};

/**
Производный от MBeeAddress класс для 16-ти битного адреса модема.
*/
class MBeeAddress16 : public MBeeAddress 
{
public:
 /**
 Конструктор "по умолчанию". Создает объект с широковещательным 16-битным адресом.
 */
 MBeeAddress16();
 
 /**
 Создает объект с заданным 16-битным адресом.
 */
 MBeeAddress16(uint16_t addr);
 
 /**
 Возвращает 16-битный адрес.
 */
 uint16_t getAddress();
 
 /**
 Устанавливает 16-битный адрес.
 */
 void setAddress(uint16_t addr);
private:
 uint16_t _addr;
};

//Подключение библиотек проектов.
#include "includes\SerialStar.h"
#include "includes\MB-ZigBee.h"

#endif //MBee_h


