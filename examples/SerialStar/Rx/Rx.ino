 /**
  * Этот файл является частью библиотеки MBee-Arduino.
  * 
  * MBee-Arduino является бесплатным программным обеспечением. Вы можете 
  * распространять и/или модифицировать его без ограничений так, как это
  * предусмотрено GNU General Public License, начиная с версии 3.
  * 
  * Надеемся, что программные продукты, созданные с помощью данной библиотеки 
  * будут полезными, однако никакие гарантии, явные или подразумеваемые не
  * предоставляются.
  * \author </i> von Boduen. Special thanx to Andrew Rapp.
  */
      
#include <MBee.h>

/**
  * Скетч предназначен для демонстрации приема пакетов с неструктурированными данными от удаленного модема.  
  * Передающий и принимающий модемы работают под управлением программного обеспечения SerialStar
  * для модулей MBee-868-x.0.
  * Действия, производимые скетчем подробно описаны в комментариях к соответствующим строкам.
  * Потребуются 2 модуля MBee-868-x.0. Первый модуль соедининяется с платой Arduino c помощью 
  * XBee-shield или любого другого совместимого устройств. Если доступного шилда нет, то возможно 
  * соединение Arduino и модуля с помощью проводов.
  * ВНИМАНИЕ!!! Модуль MBee-868-x.0 имеет номинальное значение напряжения питания 3,3В. Если Ваша
  * плата Arduino имеет выходы с логическими уровнями 5В, то необходимо предусмотреть делитель 
  * напряжения между выходом TX Arduino и входом RX модуля (вывод №3 для всех моделей). К выводу TX
  * Arduino подключается резистор 2К, с которым соединен резистор 1К, второй вывод последнего
  * сажается на землю. Точка соединения резисторов соединяется с выводом №3 модуля. 
  * Вывод №2 модуля (TX), подключается ко входу RX Arduino через последовательный резистор 1К.
  * При использовании для питания модуля собственного источника 3,3В Arduino, необходимо помнить о том,
  * что модули могут потреблять в режиме передачи токи до 200 мА. Поэтому необходимо уточнять 
  * нагрузочные характеристики применяемой Вами платы Arduino. При коротких эфирных пакетах для 
  * компенсации недостаточного выходного тока источника 3,3В можно применить конденсаторы с емкостью
  * не менее 2200 мкФ, устанавливаемые параллельно питанию модуля.
  * На обоих модулях, после загрузки программного обеспечения SerialStar, должен быть произведен возврат 
  * к заводским настройкам одним из двух способов:
  * 1. Быстрое 4-х кратное нажатие "SYSTEM BUTTON" (замыкание вывода №36 модуля на землю).
  * 2. С помощью командного режима: +++, AT RE<CR>, AT CN<CR>.
  * 
  * Первый модуль должен быть предварительно настроен для работы в пакетном режиме с escape-
  * символами AP=2. Режим аппаратного управления потоком (CTS/RTS) должен быть отключен.
  * Последовательность для настройки: +++, AT AP2<CR>, AT CN<CR>.
  * Для контроля приема данных, к Arduino должны быть подключены через резисторы 1К два светодиода.
  * 
  * Второй модуль устанавливается на плату MB-USBridge, или любой другой UART-USB/UART-COM 
  * преобразователь с выходными уровнями 3,3 В, для подключения к компьютеру. На компьютере
  * должна быть запущена любая терминальная программа с настройками порта 9600 8N1.
  * С ее помощью осуществляется передача кодов, соответствующих цифровым клавишам.
  * Данный скетч предназначет только для приема пакетов с длиной поля данных 1 байт.
  * Диапазон допустимых значений 0x31..0x39, что соответствует кнопкам 1..9.
  * При успешном приеме, светодиод statusLed будет мигать в соответствии с нажатой цифровой клавишей.  
  * Никакие дополнительные предварительные настройки второго модуля не требуются.
  */
 
SerialStar mbee = SerialStar();

RxResponse rx = RxResponse();

int errorLed = 11;
int statusLed = LED_BUILTIN; //Используется встроенный в Вашу плату Arduino светодиод.
//int statusLed = 12;

uint8_t option = 0;
uint8_t data = 0;

void setup() 
{
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);
  Serial.begin(9600);
  mbee.begin(Serial);
  delay(500); //Задержка не обязательна и вставлена для удобства работы с терминальной программой.
}

void loop() 
{
  mbee.readPacket(); //Постоянно проверяем наличие данных от модема.
  if(mbee.getResponse().isAvailable()) 
  {
    if(mbee.getResponse().getApiId() == RECEIVE_PACKET_API_FRAME || mbee.getResponse().getApiId() == RECEIVE_PACKET_NO_OPTIONS_API_FRAME) 
    {
      mbee.getResponse().getRxResponse(rx); //Получаем пакет с данными.
      //Ниже приводятся некоторые функции доступа к различным полям пакета.
      //uint16_t remoteAddress = rx.getRemoteAddress(); //Получение адреса отправителя.
      //uint8_t rssi = rx.getRssi(); //Получения уровня сигнала на входе модуля на момент приема данного пакета. Передается в виде числа со знаком в дополнительном коде.
      //bool broadcastIndicator = rx.isAddressBroadcast(); //Является ли принятый пакет широковещательным?
      //bool acknowledgeIndicator = rx.isAcknowledged(); //Было ли отправлено подтверждение приема данного пакета? 
      if(rx.getDataLength() == 1) //Ожидаем, что передается только 1 байт.
      {
        data = rx.getData()[0]; //Считываем принятый байт данных.
        if((data > '0') && (data <= '9')) //Проверяем, находится ли принятый байт в заданном диапазоне.  
          flashLed(statusLed, data - '0', 200); //Моргаем светодиодом в соответствии с нажатой цифровой клавишей.
      }
    }
    else
      flashLed(errorLed, 1, 500); //Принят фрейм, не предназначенный для передачи неструктурированных данных.   
  } 
  else if(mbee.getResponse().isError()) 
    flashLed(errorLed, 2, 100); //При разборе принятого пакета произошли ошибки.  
}

void flashLed(int pin, int times, int wait) 
{
  for (int i = 0; i < times; i++) 
  {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);
    if (i + 1 < times)
      delay(wait);
  }
}
