**DPDK telemetry test**


_удобный потоковый класс для работы с телеметрией._

usage: 

 * Пример использования #1
 *   DPDKTelemetry tm;
 *   tm << "/ethdev/link_status,0";
 *   std::cout << "link of the port #0 is " << tm["/ethdev/link_status.status"] << std::endl;
 *   будет отображено 'link of the port #0 is UP'
 *
 * Пример использования #2
 *   std::string devices;
 *   tm << "/ethdev/list" >> devices;
 *    В devices будет json ответ с перечнем устройств. 
 *
 *   Команды и ответы смотреть на устройстве через консольную утилиту dpdk-telemetry
 *   
 *    Больше информации: http://doc.dpdk.org/guides/howto/telemetry.html
