#include  "dpdk-telemetry.h"

int main(int argc, char **argv)
{
     std::string path = "/var/run/dpdk/rte/dpdk_telemetry.v2";
     if(argc > 1)
     {
          path = argv[1];
     }

     DPDKTelemetry telemetry(path);
     std::string ret;
     telemetry << "/ethdev/link_status,0" >> ret;
     std::cout << "iface #0 state: " << telemetry["/ethdev/link_status.status"] << std::endl;
     std::cout << "whole message: '" << ret << "' \n";
     return EXIT_SUCCESS;
}
