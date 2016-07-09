#include "udpsocket.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

namespace sonic_socket
{

UdpSocket::UdpSocket()
    : file_desc(-1)
{
}

void UdpSocket::open(Remote::Port port)
{
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

void UdpSocket::close()
{
    shutdown(file_desc, SHUT_RDWR);
    ::close(file_desc);
    file_desc = -1;
}

void UdpSocket::send(const Remote &remote, const char *data, unsigned int data_len) const
{
    if (file_desc < 0) {return;}

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

void UdpSocket::poll(Remote &remote, char *data, unsigned int &data_len, unsigned int max_len) const
{
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

}
