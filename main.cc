#include  "dpdk-telemetry.h"
#include <iostream>

int main(int argc, char **argv)
{
     std::string path = "/var/run/dpdk/rte/dpdk_telemetry.v2";
     if(argc > 1)
     {
          path = argv[1];
     }

     std::string status;
     DPDKTelemetry telemetry(path);
     telemetry << "/ethdev/link_status,0" >> status;
     std::cout << "iface #0 state: " << telemetry["/ethdev/link_status.status"] << std::endl;
     std::cout << "whole message: '" << status << "' \n";
     return EXIT_SUCCESS;
}
