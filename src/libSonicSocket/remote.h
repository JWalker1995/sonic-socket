#ifndef SONICSOCKET_REMOTE_H
#define SONICSOCKET_REMOTE_H

#include <cstddef>
#include <netinet/in.h>
#include <string>
#include <exception>
#include <assert.h>

namespace sonic_socket
{

// TODO: IPv6 compatability

class Remote
{
    friend class UdpSocket;

public:
    typedef std::uint_fast16_t Port;

    struct Hasher
    {
        std::size_t operator()(const Remote &remote) const
        {
            std::size_t res = 0;
            res ^= remote.addr.sin_family;
            res ^= remote.addr.sin_port;
            res ^= remote.addr.sin_addr.s_addr;
            return res;
        }
    };

    class DnsException : public std::exception
    {
        friend class Remote;

    public:
        virtual const char *what() const noexcept
        {
            return str.c_str();
        }

    private:
        DnsException(const std::string &host, int getaddrinfo_return, int error_code)
            : str("Could not resolve the hostname \"" + host + "\" (getaddrinfo() -> " + std::to_string(getaddrinfo_return) + ", errno = " + std::to_string(error_code) + ")")
        {}

        std::string str;
    };


    Remote()
    {}

    void resolve(const std::string &host, Port port);

    bool is_resolved() const {return addr_len;}

    bool operator==(const Remote &other) const
    {
        assert(addr_len == other.addr_len);

        return addr.sin_family == other.addr.sin_family
            && addr.sin_port == other.addr.sin_port
            && addr.sin_addr.s_addr == other.addr.sin_addr.s_addr;
    }

    std::string to_string() const;

private:
    struct sockaddr_in addr;
    socklen_t addr_len = 0;
};

}

#endif // SONICSOCKET_REMOTE_H
