#ifndef SONICSOCKET_UDPSERVER_H
#define SONICSOCKET_UDPSERVER_H

#include <exception>
#include <unistd.h>
#include <netinet/in.h>

#include "libSonicSocket/config/SS_UDPSOCKET_DROP_ENABLE.h"
#ifdef NDEBUG
#undef SS_UDPSOCKET_DROP_ENABLE
#define SS_UDPSOCKET_DROP_ENABLE 0
#endif

#if SS_UDPSOCKET_DROP_ENABLE
#include <random>
// Include <chrono> in case SS_UDPSOCKET_DROP_SEED is std::chrono::system_clock::now().time_since_epoch().count()
#include <chrono>
#include <limits>
#include "libSonicSocket/config/SS_UDPSOCKET_DROP_PROBABILITY.h"
#include "libSonicSocket/config/SS_UDPSOCKET_DROP_SEED.h"
#endif

#include "libSonicSocket/remote.h"

#define UDPSERVER_MESSAGE_MAX_SIZE 512

namespace sonic_socket
{

class UdpSocket
{
public:
    UdpSocket()
        : file_desc(-1)
    #if SS_UDPSOCKET_DROP_ENABLE
        , drop_gen(SS_UDPSOCKET_DROP_SEED)
    #endif
    {}

    void open(Remote::Port port) {
        assert(file_desc == -1);

        file_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (file_desc < 0)
        {
            file_desc = -1;
            throw SocketException(errno);
        }

        struct sockaddr_in local_addr;
        memset((char *)&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        local_addr.sin_port = htons(port);

        if (bind(file_desc, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
        {
            file_desc = -1;
            throw BindException(errno);
        }
    }

    void close() {
        shutdown(file_desc, SHUT_RDWR);
        ::close(file_desc);
        file_desc = -1;
    }

    void send(const Remote &remote, const char *data, unsigned int data_len) {
        if (file_desc < 0) {return;}

    #if SS_UDPSOCKET_DROP_ENABLE
        if (std::generate_canonical<float, std::numeric_limits<float>::digits>(drop_gen) < static_cast<float>(SS_UDPSOCKET_DROP_PROBABILITY))
        {
            throw DropException();
        }
    #endif

        ssize_t res = sendto(file_desc, data, data_len, MSG_DONTWAIT, reinterpret_cast<const struct sockaddr*>(&remote.addr), remote.addr_len);
        if (res < 0)
        {
            //assert(res == -1);
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // TODO: What to do here?
                assert(false);

                /*
                std::vector<char>::size_type prev_size = send_queue.size();
                send_queue.resize(prev_size + data_len);
                std::copy(data, data + data_len, send_queue.begin() + prev_size);
                */
            }
            else
            {
                throw SendException(remote, errno);
            }
        }
        else
        {
            assert(res == data_len);
        }
    }

    void poll(Remote &remote, char *data, unsigned int &data_len, unsigned int max_len) const {
        if (file_desc < 0) {return;}

        remote.addr_len = sizeof(remote.addr);

        data_len = recvfrom(file_desc, data, max_len, MSG_WAITALL, reinterpret_cast<struct sockaddr*>(&remote.addr), &remote.addr_len);
        if (data_len == static_cast<unsigned int>(-1))
        {
            throw PollException(errno);
        }

        assert(data_len <= max_len);
        assert(remote.addr_len <= sizeof(remote.addr));
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

    class DropException : public std::exception
    {
        friend class UdpSocket;

    public:
        virtual const char *what() const noexcept
        {
            return "Dropped packet (to disable dropping, set SS_UDPSOCKET_DROP_ENABLE to 0)";
        }

    private:
        DropException()
        {}
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

#if SS_UDPSOCKET_DROP_ENABLE
    std::default_random_engine drop_gen;
#endif
};

}

#endif // SONICSOCKET_UDPSERVER_H
