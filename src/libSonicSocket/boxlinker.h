#ifndef BOXLINKER_H
#define BOXLINKER_H

#include <string>
#include <unordered_map>

#include "inbox.h"
#include "outbox.h"

namespace sonic_socket {

class BoxLinker {
    // Links inboxes and outboxes to remote pairings.
    // Service queries can return this.
    // Can be created from a raw BoxLinker message.
    // Possibly delayed.

public:
    template <
        typename _MessageType,
        bool (*parse_callback)(const _MessageType &, std::string &)
        >
    void save(Inbox<_MessageType, parse_callback> &inbox) {
        link<_MessageType>(_MessageType::descriptor()->full_name(), inbox);
    }

    template <
        typename _MessageType,
        bool (*parse_callback)(const _MessageType &, std::string &)
        >
    void save(const std::string &key, Inbox<_MessageType, parse_callback> &inbox) {

    }

    template <
        typename _MessageType
        >
    void link(Outbox<_MessageType> &outbox) {
        link<_MessageType>(_MessageType::descriptor()->full_name(), outbox);
    }

    template <
        typename _MessageType
        >
    void link(const std::string &key, Outbox<_MessageType> &outbox) {

    }

private:
};

}

#endif // BOXLINKER_H
