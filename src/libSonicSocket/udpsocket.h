#ifndef SONICSOCKET_UDPSERVER_H
#define SONICSOCKET_UDPSERVER_H

#include <exception>
#include <netinet/in.h>

#include "libSonicSocket/remote.h"

#define UDPSERVER_MESSAGE_MAX_SIZE 512

namespace sonic_socket
{

class UdpSocket
{
public:
    UdpSocket();

    void open(Remote::Port port);
    void close();

    void send(const Remote &remote, const char *data, unsigned int data_len) const;
    void poll(Remote &remote, char *data, unsigned int &data_len, unsigned int max_len) const;

    void test()
    {
        //UdpServer server;
        //server.open(123);
        //server.make_remote();
    }


    class SocketException : public std::exception
    {
        friend class UdpSocket;

    public:
        virtual const char *what() const noexcept
        {
            return str.c_str();
        }

    private:
        SocketException(int error_code)
            : str("Could not create socket (errno = " + std::to_string(error_code) + ")")
        {}

        std::string str;
    };

    class BindException : public std::exception
    {
        friend class UdpSocket;

    public:
        virtual const char *what() const noexcept
        {
            return str.c_str();
        }

    private:
        BindException(int error_code)
            : str("Cannot bind to socket (errno = " + std::to_string(error_code) + ")")
        {}

        std::string str;
    };

    class SendException : public std::exception
    {
        friend class UdpSocket;

    public:
        virtual const char *what() const noexcept
        {
            return str.c_str();
        }

    private:
        SendException(const Remote &remote, int error_code)
            : str("Could not send packet to remote \"" + remote.to_string() + "\" (errno = " + std::to_string(error_code) + ")")
        {}

        std::string str;
    };

    class PollException : public std::exception
    {
        friend class UdpSocket;

    public:
        virtual const char *what() const noexcept
        {
            return str.c_str();
        }

    private:
        PollException(int error_code)
            : str("Could not poll (errno = " + std::to_string(error_code) + ")")
        {}

        std::string str;
    };

private:
    int file_desc;
};

}

#endif // SONICSOCKET_UDPSERVER_H
