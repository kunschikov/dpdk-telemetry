**DPDK telemetry test**


_удобный потоковый класс для работы с телеметрией._

usage: 
   
   DPDKTelemetry dt;
   dt << "/ethdev/list" >> iface_list << "/ethdev/link_status,1" >> link_status;

   
