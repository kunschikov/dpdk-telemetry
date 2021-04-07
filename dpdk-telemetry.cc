#include "dpdk-telemetry.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <syslog.h>
#include <poll.h>

#include <unistd.h>
#include <boost/property_tree/ptree.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#include <boost/property_tree/json_parser.hpp>
#pragma GCC diagnostic pop

DPDKTelemetry::DPDKTelemetry(const std::string& p)
{
     if(not p.empty())
     {
          path = p;
     }
}


DPDKTelemetry::~DPDKTelemetry()
{
     close();
}


int DPDKTelemetry::open()
{
     int fd = ::socket(AF_UNIX, SOCK_SEQPACKET, 0);
     if(fd == -1)
     {
          ::syslog(LOG_ERR, "failed to create socket for the telemetry connection: %s", strerror(errno));
          return -1;
     }
     struct sockaddr_un su;
     su.sun_family = AF_LOCAL;
     strcpy(su.sun_path, path.c_str());

     if(-1 == ::connect(fd, (struct sockaddr*)&su, sizeof(struct sockaddr_un)))
     {
          ::syslog(LOG_ERR, "failed to connect socket to the '%s' telemetry connection: %s", path.c_str(), strerror(errno));
          ::close(fd);
          return -1;
     }

     ::syslog(LOG_INFO, "Connected to the '%s' telemetry socket", path.c_str());
     return fd;
}


void DPDKTelemetry::close()
{
     if(-1 == fd)
     {
          return;
     }

     if(::close(fd) == -1)
     {
          syslog(LOG_ERR, "Errors while shutting down telemetry connection fd #%d: %s", fd, strerror(errno));
     }

     ::syslog(LOG_INFO, "Connection to the '%s' telemetry socket was closed.", path.c_str());
     fd = -1;
}


bool DPDKTelemetry::check()
{
     if(fd == -1)
     {
          fd = open();
          status = read();
          ::syslog(LOG_INFO, "'%s' telemetry socket info: %s", path.c_str(), status.c_str());
     }

     return fd != -1;
}


bool DPDKTelemetry::write(const std::string& message)
{
     reply.clear();
     if(not check())
     {
          ::syslog(LOG_WARNING, "refused to write command '%s' to the telemetry connection: it is not valid", message.c_str());
          return false;
     }

     ssize_t count = ::send(fd, message.c_str(), message.size(), 0);
     if(-1 == count)
     {
          ::syslog(LOG_ERR, "failed to write command '%s' to the telemetry connection: %s", message.c_str(), strerror(errno));
          close();
          return false;
     }

     if(count != (ssize_t) message.size())
     {
          ::syslog(LOG_WARNING, "failed to write command '%s' to the telemetry connection: only %lu from %lu bytes were written", message.c_str(), count, message.size());
          return false;
     }

     return true;
}


std::string DPDKTelemetry::read()
{
     if(not check())
     {
          ::syslog(LOG_WARNING, "refused to read from the '%s' telemetry socket: it is not valid", path.c_str());
          return "";
     }

     std::string ret;
     const int timeout = 20;
     for(int i = 0; i < timeout; i++)
     {
          ssize_t count = 0;
          char buffer[1024];
          struct pollfd pfd{fd, POLLIN, 0};
          count = poll(&pfd, 1, 1000);
          if(count == -1)
          {
               ::syslog(LOG_ERR, "failed to poll the %s. Got '%s' output and '%s' system error", path.c_str(), ret.c_str(), strerror(errno));
               break;
          }

          if(not count or not (pfd.revents & POLLIN))
          {
               continue;
          }

          count = ::recv(fd, buffer, sizeof(buffer), 0);
          if(-1 == count)
          {
               ::syslog(LOG_ERR, "failed to read from the %s. Got '%s' output and '%s' system error", path.c_str(), ret.c_str(), strerror(errno));
               break;
          }

          if(not count)
          {
               ::syslog(LOG_INFO, "failed to read from the %s. Got '%s' output and other side closed connection.", path.c_str(), ret.c_str());
               break;
          }
          ret.append(buffer, count);

          if(buffer[count - 1] == '}' or buffer[count - 1] == 0)
          {
               //заканчиваем чтение
               break;
          }
     }


     return ret;
}

DPDKTelemetry& DPDKTelemetry::operator << (const std::string& command)
{
     write(command);
     return *this;
}


DPDKTelemetry& DPDKTelemetry::operator >> (std::string& user_buffer)
{
     reply = user_buffer = read();
     return *this;
}


std::string DPDKTelemetry::operator [] (const std::string& key)
{
     if(reply.empty())
     {
          reply = read();
     }

     if(reply.empty())
     {
          ::syslog(LOG_WARNING, "failed to read from the the telemetry connection. Can't get '%s' key value", key.c_str());
          return "";
     }

     if(reply == "null")
     {
          ::syslog(LOG_INFO, "'%s' telemetry key value is 'null'", key.c_str());
          return "";
     }

     try
     {
          std::stringstream iface_status_stream;
          iface_status_stream << reply;

          boost::property_tree::ptree pt_status;
          boost::property_tree::read_json(iface_status_stream, pt_status);
          return pt_status.get_child(key).data();
     }
     catch(const boost::property_tree::ptree_error &e)
     {
          ::syslog(LOG_WARNING, "'%s' key from the '%s' property error: %s", key.c_str(), reply.c_str(), e.what());
     }
     catch (std::exception const& e)
     {
          ::syslog(LOG_WARNING, "'%s' key from the '%s' standard error: %s", key.c_str(), reply.c_str(), e.what());
     }
     return "";
}


std::string DPDKTelemetry::version()
{
     if(not check())
     {
          ::syslog(LOG_WARNING, "refused to read from the '%s' telemetry socket: it is not valid", path.c_str());
          return "";
     }
     return status;
}


DPDKTelemetry::operator std::string()
{
     return reply;
}
