#include "dpdk-telemetry.h"
#include <iostream>

int main(int argc, char **argv)
{
     std::string path = "/var/run/dpdk/rte/dpdk_telemetry.v2";
     if(argc > 1)
     {
          path = argv[1];
     }

     std::string devices;
     DPDKTelemetry tm;
     tm << "/ethdev/list" >> devices;
     std::cout << "device list " << devices;

     for(const auto &device: DPDKTelemetry("/ethdev/list").as_vector())
         std::cout << "iface: " << device << " link: " << DPDKTelemetry("/ethdev/link_status," + device) << std::endl;
     return EXIT_SUCCESS;
}
