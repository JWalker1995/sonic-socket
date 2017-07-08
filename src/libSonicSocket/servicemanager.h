#ifndef SONICSOCKET_SERVICEMANAGER_H
#define SONICSOCKET_SERVICEMANAGER_H

#include <string>
#include <algorithm>
#include <unordered_set>

#include "libSonicSocket/jw_util/methodcallback.h"

#include "libSonicSocket/servicequery.h"
#include "libSonicSocket/mailbox.h"
#include "libSonicSocket/base.pb.h"

namespace sonic_socket {

class ServiceManager {
public:
    template <typename... BoxTypes>
    void publish_multiple(ServiceQuery &&query, jw_util::MethodCallback<const MailboxInit &> callback) {
        query.prepare();
        published_services.insert(PublishedService{std::move(query.data), false, callback});
    }

    template <typename... BoxTypes>
    void publish_single(ServiceQuery &&query, Mailbox<BoxTypes...> &mailbox) {
        query.prepare();
        published_services.insert(PublishedService{std::move(query.data), true, create_mailbox_init_callback(mailbox)});
    }

    template <typename... BoxTypes>
    void subscribe(ServiceQuery &&query, Mailbox<BoxTypes...> &mailbox) {
        query.prepare();
        published_services.insert(PublishedService{std::move(query.data), true, create_mailbox_init_callback(mailbox)});
    }

    float find_match(ServiceQuery &&query) {
    }

private:
    struct PublishedService {
        const internal::ServiceQuery query;
        bool single;
        jw_util::MethodCallback<const MailboxInit &> callback;

        struct Hasher {
            std::size_t operator()(const PublishedService &service) const {
                return std::hash<std::string>()(service.query.interface());
            }
        };

        bool operator==(const PublishedService &other) const {
            return query.interface() == other.query.interface();
        }
    };

    std::unordered_multiset<PublishedService, PublishedService::Hasher> published_services;

    static float calc_query_match(const internal::ServiceQuery &q1, const internal::ServiceQuery &q2) {
        assert(q1.interface() == q2.interface());

        float score = 0.0f;

        google::protobuf::RepeatedPtrField<internal::ServiceQueryCoordinate>::const_iterator i1 = q1.coords().cbegin();
        google::protobuf::RepeatedPtrField<internal::ServiceQueryCoordinate>::const_iterator i2 = q2.coords().cbegin();

        while (i1 != q1.coords().cend() && i2 != q2.coords().cend()) {
            if (i1->key() < i2->key()) {
                i1++;
            } else if (i1->key() > i2->key()) {
                i2++;
            } else {
                score += calc_query_coord_match(*i1, *i2);
            }
        }

        return score;
    }

    static float calc_query_coord_match(const internal::ServiceQueryCoordinate &c1, const internal::ServiceQueryCoordinate &c2) {
        assert(c1.key() == c2.key());

        float weight = c1.weight() + c2.weight();

        if (!c1.string_value().empty() && !c2.string_value().empty()) {
            return c1.string_value() == c2.string_value() ? weight : 0.0f;
        } else {
            float error = c1.float_value() - c2.float_value();
            return weight * error * error;
        }
    }

    template <typename... BoxTypes>
    static jw_util::MethodCallback<const MailboxInit &> create_mailbox_init_callback(Mailbox<BoxTypes...> &mailbox) {
        return jw_util::MethodCallback<const MailboxInit &>::create<&Mailbox<BoxTypes...>::connect>(&mailbox);
    }

    void subscribe_to_local(std::unordered_multiset<PublishedService, PublishedService::Hasher>::const_iterator service_i, const MailboxInit &mailbox_init) {
        service_i->callback.call(mailbox_init);

        if (service_i->single) {
            published_services.erase(service_i);
        }
    }
};

}
#endif // SONICSOCKET_SERVICEMANAGER_H
