#include "dpdk-telemetry.h"

#include <sys/stat.h>                               //::stat
#include <sys/socket.h>                             //::socket
#include <sys/un.h>                                 //sockaddr_un
#include <syslog.h>                                 //::syslog
#include <poll.h>                                   //::poll

#include <boost/property_tree/json_parser.hpp>      //property_tree::read_json

DPDKTelemetry::DPDKTelemetry(const std::string& q, const std::string& p):query(q)
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
    struct stat s;
    if(0 != ::stat(path.c_str(), &s))
     {
          return -1;
     }

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

     fd = -1;
}


bool DPDKTelemetry::check()
{
     if(fd == -1)
     {
          fd = open();
     }

     if(fd == -1)
     {
          return false;
     }

     status = read();
     return true;
}


bool DPDKTelemetry::write(const std::string& message)
{
     reply.clear();
     if(not check())
     {
          return false;
     }

     ssize_t count = ::send(fd, message.c_str(), message.size(), 0);
     if(-1 == count)
     {
          close();
          return false;
     }

     return  count == (ssize_t) message.size();
}


std::string DPDKTelemetry::read()
{
     std::string ret;
     const int timeout = 60;
     for(int i = 0; i < timeout;)
     {
          ssize_t count = 0;
          char buffer[65536];
          struct pollfd pfd{fd, POLLIN, 0};

          count = poll(&pfd, 1, 1000);
          if(count == -1)
          {
               ::syslog(LOG_ERR, "failed to poll the %s. Got '%s' output and '%s' system error", path.c_str(), ret.c_str(), strerror(errno));
               break;
          }

          if(not count or not (pfd.revents & POLLIN))
          {
               i++;
               continue;
          }

          count = ::read(fd, buffer, sizeof(buffer));
          if(-1 == count)
          {
               break;
          }

          if(not count)
          {
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
     if(fd != -1)
     {
          reply = user_buffer = read();
     }
     return *this;
}


std::string DPDKTelemetry::operator [] (const std::string& key)
{
     if(fd == -1)
     {
          return "";
     }
     if(reply.empty())
     {
          reply = read();
     }

     if(reply.empty())
     {
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


DPDKTelemetry:: operator bool()
{
     return fd != -1;
}
