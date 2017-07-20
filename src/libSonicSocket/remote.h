#ifndef SONICSOCKET_REMOTE_H
#define SONICSOCKET_REMOTE_H

#include <cstddef>
#include <netdb.h>
#include <arpa/inet.h>
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

    void resolve(const std::string &host, Port port) {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_flags = 0;

        struct addrinfo *found_addresses;
        int res = getaddrinfo(host.c_str(), NULL, &hints, &found_addresses);
        if (res != 0)
        {
            throw DnsException(host, res, errno);
        }

        struct addrinfo *found_addr;
        for (found_addr = found_addresses; found_addr != NULL; found_addr = found_addr->ai_next)
        {
            if (found_addr->ai_socktype == SOCK_DGRAM && found_addr->ai_protocol == IPPROTO_UDP)
            {
                break;
            }
        }

        if (found_addr != NULL)
        {
            assert(found_addr->ai_family == found_addr->ai_addr->sa_family);

            memcpy(&addr, found_addr->ai_addr, found_addr->ai_addrlen);
            addr.sin_port = htons(port);
            addr_len = found_addr->ai_addrlen;

            freeaddrinfo(found_addresses);
        }
        else
        {
            freeaddrinfo(found_addresses);

            throw DnsException(host, 0, 0);
        }
    }

    bool is_resolved() const {return addr_len;}

    bool operator==(const Remote &other) const
    {
        assert(addr_len == other.addr_len);

        return addr.sin_family == other.addr.sin_family
            && addr.sin_port == other.addr.sin_port
            && addr.sin_addr.s_addr == other.addr.sin_addr.s_addr;
    }

    std::string to_string() const {
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN);
        unsigned int port = ntohs(addr.sin_port);
        return std::string(str) + ":" + std::to_string(port);
    }

private:
    struct sockaddr_in addr;
    socklen_t addr_len = 0;
};

inline std::string to_string(const Remote &remote)
{
    return remote.to_string();
}

}

#endif // SONICSOCKET_REMOTE_H
