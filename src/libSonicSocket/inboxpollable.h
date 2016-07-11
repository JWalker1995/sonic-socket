#ifndef SONICSOCKET_INBOXPOLLABLE_H
#define SONICSOCKET_INBOXPOLLABLE_H

#include "libSonicSocket/config/SS_INBOXPOLLABLE_USE_READERWRITERQUEUE.h"

#include "libSonicSocket/inbox.h"

#if SS_INBOXPOLLABLE_USE_READERWRITERQUEUE
#include "readerwriterqueue/readerwriterqueue.h"
#else
#include <mutex>
#include <queue>
#endif

namespace sonic_socket
{

template <
    typename _MessageType,
    bool (*parse_callback)(const _MessageType &, std::string &)
>
class InboxPollable : public Inbox<_MessageType, parse_callback>
{
public:
    typedef _MessageType MessageType;
    typedef InboxPollable<_MessageType, parse_callback> SelfType;

    template <typename MailboxType>
    class Actor : public Inbox<_MessageType, parse_callback>::template Actor<MailboxType>
    {
    public:
        void register_and_generate_mailbox_init(MailboxInit &mailbox_init)
        {
            register_and_generate_mailbox_init(mailbox_init, make_inbox_registration());
        }

        void unregister()
        {
            unregister(make_inbox_registration());
        }

    private:
        MessageRouter::RegisteredInbox make_inbox_registration()
        {
            return MailboxType::template get_mailbox<SelfType>(this).template generate_inbox_registration<SelfType>();
        }

#ifdef SS_INBOXPOLLABLE_USE_READERWRITERQUEUE

    public:
        class Reader
        {
            friend class InboxPollable;

        public:
            Reader(InboxPollable &inbox)
#ifndef NDEBUG
                : top(0)
#endif
            {}

        private:
            MessageType *top;
        };

        bool has_message(Reader &reader)
        {
            reader.top = queue.peek();
            return reader.top != 0;
        }

        const MessageType &get_message(Reader &reader) const
        {
            assert(reader.top != 0);
            return *reader.top;
        }

        void pop_message(Reader &reader)
        {
            bool res = queue.pop();
            assert(res);
        }

    private:
        moodycamel::ReaderWriterQueue<MessageType> queue;

        void process_message(const MessageType &message)
        {
            bool success = queue.enqueue(std::move(message));
            if (!success)
            {
                throw std::bad_alloc();
            }
        }

#else

    public:
        class Reader
        {
            friend class InboxPollable;

        public:
            Reader(InboxPollable &inbox)
                : lock(inbox.queue_mutex)
            {}

        private:
            std::lock_guard<std::mutex> lock;
        };

        bool has_message(Reader &reader)
        {
            (void) reader;
            return !queue.empty();
        }

        const MessageType &get_message(Reader &reader) const
        {
            (void) reader;
            return queue.front();
        }

        void pop_message(Reader &reader)
        {
            (void) reader;
            queue.pop();
        }

    private:
        std::mutex queue_mutex;
        std::queue<MessageType> queue;

        void process_message(const MessageType &message)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            (void) lock;
            queue.push(std::move(message));
        }
#endif
    };
};

}

#endif // SONICSOCKET_INBOXPOLLABLE_H
