#pragma once
#ifndef __DPDK_TELEMETRY_H__
#define __DPDK_TELEMETRY_H__
#include <string>
/* *
 *   Работа с телеметрией DPDK как с потоком с++
 *
 * Пример использования #0
 *   std::cout << "link of the port #0 is " << DPDKTelemetry("/ethdev/link_status,0")["/ethdev/link_status.status"] << std::endl;
 *   будет отображено 'link of the port #0 is UP' или DOWN
 *
 * Пример использования #1
 *   std::string devices, link;
 *   DPDKTelemetry tm;
 *   tm << "/ethdev/list" >> devices << "/ethdev/link_status,0" >> link;
 *   std::cout << "device lis is: " << devices << " link of the port #0 is " << link << std::endl;
 *
 *   Команды и ответы смотреть на устройстве через консольную утилиту dpdk-telemetry
 *   
 *    Больше информации: http://doc.dpdk.org/guides/howto/telemetry.html
 * */

class DPDKTelemetry
{
public:
          ///@brief создаем объект соединения без фактического соединения к телеметрии
          ///@param query запрос, который будет послан в телеметрию при фактическом задействовании
          ///@param path опциональный путь до сокета телеметрии DPDK
          explicit DPDKTelemetry(const std::string& query = "", const std::string& path = "");

          ///@brief закрываем соединение по необходимости
          ~DPDKTelemetry();

          ///@brief оператор отправки запроса в телеметрию 
          //   1. шлем команду 'command' в телеметрию 
          ///@param command команда телеметрии 
          DPDKTelemetry& operator << (const std::string& command);

          ///@brief оператор чтения ответа телеметрии. Ответ пишется как в буфер, так и во внутреннее хранилище
          ///@param buffer строковый буфер, куда будет скопирован ответ
          DPDKTelemetry& operator >> (std::string& buffer);

          ///@brief оператор поиска в крайнем ответе телеметрии значения по ключу.
          /// При пустом внутреннем буфере производится попытка чтения сокета.
          ///@param key ключ, по которому производится поиск
          ///@return значение по ключу или пустая строка в случае отсутствия ключа/данных
          std::string operator [] (const std::string& key);

          ///@brief возвращает версию и статус DPDK приложения, с которым работает.
          ///@return первое сообщение после коннекта, в котором отсылается json -  описатель системы
          std::string version();

          ///@brief конверсия в std::string 
          ///@return возвращаем крайний ответ
          operator std::string();

          ///@brief статус: есть ли соединение к телеметрии
          ///@return возвращаем true при наличии коннекта
          operator bool();
private:
          int fd = -1;                                              // канал обмена данными с телеметрией
          std::string path = "/var/run/dpdk/rte/dpdk_telemetry.v2"; // путь к сокету DPDK
          std::string status;                                       // первый ответ: версия тулкита, pid приложения 
          std::string query;                                        // крайний запрос
          std::string reply;                                        // крайний ответ DPDK

          ///@brief открывает сокет, по которому DPDK пишет телеметрию
          ///@return socket FD или -1 по невозможности открыть соединение.
          int open();

          ///@brief закрывает соединение если открыто
          void close();

          ///@brief пишет message в сокет fd, при необходимости переоткрывает соединение
          ///@param message сообщение в телеметрию
          ///@return возвращает статус операции: true при успешности, false по неудаче
          bool write(const std::string& message);

          ///@brief проверяет статус сокета, переоткрывает по необходимости
          ///@return возвращает статус операции: true при успешности, false по неудаче
          bool check();

          ///@brief выполняет чтение из сокета fd, возвращает прочитанные данные,при  необходимости переоткрывает соединение
          ///@return полученный ответ
          std::string read();
};
#endif // __DPDK_TELEMETRY_H__
