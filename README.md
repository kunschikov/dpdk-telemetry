**DPDK telemetry test**


_удобный потоковый класс для работы с телеметрией._

usage: 
 *   Работа с телеметрией DPDK на C++
 *
 * Пример использования #0
 *   DPDKTelemetry tm; //создан объект без открытия соединения
 *   tm << "/ethdev/link_status,0"; // открывается соединение, шлется запрос статуса сетевого порта 0
 *   std::string link = tm["/ethdev/link_status.status"];   // из ответного JSON вычитывем поле ethdev -> link_status ->status
 *  То же самое с задействованием временного объекта:
 *   std::cout << "link of the port #0 is " << DPDKTelemetry("/ethdev/link_status,0")["/ethdev/link_status.status"] << std::endl;
 *   будет отображено 'link of the port #0 is UP'
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
