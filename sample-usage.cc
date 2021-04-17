#include  "dpdk-telemetry.h"
#include <iostream>

int main(int argc, char **argv)
{
     std::string path = "/var/run/dpdk/rte/dpdk_telemetry.v2";
     if(argc > 1)
     {
          path = argv[1];
     }
     std::cout << "device list " << DPDKTelemetry("/ethdev/list", path) << std::endl;
     std::string devices, link;
     DPDKTelemetry tm;
     tm << "/ethdev/list" >> devices << "/ethdev/link_status,0" >> link;
     std::cout << "device list " << devices << " link: " << link << std::endl;
     return EXIT_SUCCESS;
}
