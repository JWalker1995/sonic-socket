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

    class ActorData
    {
        friend class InboxPollable<MessageType, parse_callback>::Reader;

    protected:
#ifdef SS_INBOXPOLLABLE_USE_READERWRITERQUEUE
        moodycamel::ReaderWriterQueue<MessageType> queue;
#else
        std::queue<MessageType> queue;
        std::mutex queue_mutex;
#endif
    };

    template <typename MailboxType>
    class Actor : public Inbox<_MessageType, parse_callback>::template Actor<MailboxType>, public ActorData
    {
        typedef typename Inbox<_MessageType, parse_callback>::template Actor<MailboxType> BaseType;

    public:
        void register_and_generate_mailbox_init(MailboxInit &mailbox_init)
        {
            BaseType::register_and_generate_mailbox_init(mailbox_init, make_inbox_registration());
        }

        void unregister()
        {
            BaseType::unregister(make_inbox_registration());
        }

#ifdef SS_INBOXPOLLABLE_USE_READERWRITERQUEUE
        void process_message(const MessageType &message)
        {
            bool success = ActorData::queue.enqueue(std::move(message));
            if (!success)
            {
                throw std::bad_alloc();
            }
        }
#else
        void process_message(const MessageType &message)
        {
            std::lock_guard<std::mutex> lock(ActorData::queue_mutex);
            (void) lock;
            ActorData::queue.push(std::move(message));
        }
#endif

    private:
        MessageRouter::RegisteredInbox make_inbox_registration()
        {
            return MailboxType::template get_mailbox<SelfType>(this).template generate_inbox_registration<SelfType>();
        }
    };

#ifdef SS_INBOXPOLLABLE_USE_READERWRITERQUEUE

    class Reader
    {
    public:
        Reader(ActorData &actor)
            : queue(actor.queue)
#ifndef NDEBUG
            , top(0)
#endif
        {}

        bool has_message()
        {
            top = queue.peek();
            return top != 0;
        }

        const MessageType &get_message() const
        {
            assert(top != 0);
            return *top;
        }

        void pop_message()
        {
            bool res = queue.pop();
            assert(res);
        }

    private:
        moodycamel::ReaderWriterQueue<MessageType> &queue;
        MessageType *top;
    };

#else

    class Reader
    {
    public:
        Reader(ActorData &actor)
            : queue(actor.queue)
            , lock(actor.queue_mutex)
        {}

        bool has_message()
        {
            return !queue.empty();
        }

        const MessageType &get_message() const
        {
            return queue.front();
        }

        void pop_message()
        {
            queue.pop();
        }

    private:
        std::queue<MessageType> &queue;
        std::lock_guard<std::mutex> lock;
    };

#endif
};

}

#endif // SONICSOCKET_INBOXPOLLABLE_H
