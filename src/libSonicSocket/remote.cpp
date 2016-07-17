#include "remote.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

namespace sonic_socket
{

void Remote::resolve(const std::string &host, Port port)
{
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

std::string Remote::to_string() const
{
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN);
    unsigned int port = ntohs(addr.sin_port);
    return std::string(str) + ":" + std::to_string(port);
}

}
