#ifndef SONICSOCKET_SERVICEQUERY_H
#define SONICSOCKET_SERVICEQUERY_H

#include <string>
#include <algorithm>

#include "internal.pb.h"

namespace sonic_socket {

class ServiceQuery {
public:
    friend class ServiceManager;

    ServiceQuery(const std::string &interface) {
        data.set_interface(interface);
    }

    void add_string(const std::string &key, float weight, const std::string &value) {
        assert(!has_key(key));
        internal::ServiceQueryCoordinate *coord = data.add_coords();
        coord->set_key(key);
        coord->set_weight(weight);
        coord->set_string_value(value);
        sorted = false;
    }

    void add_float(const std::string &key, float weight, float value) {
        assert(!has_key(key));
        internal::ServiceQueryCoordinate *coord = data.add_coords();
        coord->set_key(key);
        coord->set_weight(weight);
        coord->set_float_value(value);
        sorted = false;
    }

    bool has_key(const std::string &key) const {
        return find_key(key) != 0;
    }

    const std::string &get_string(const std::string &key) const {
        return find_key(key)->string_value();
    }

    float get_float(const std::string &key) const {
        return find_key(key)->float_value();
    }

private:
    bool sorted = true;
    internal::ServiceQuery data;

    internal::ServiceQueryCoordinate *find_key(const std::string &key) {
        google::protobuf::RepeatedPtrField<internal::ServiceQueryCoordinate>::const_iterator i = data.coords().cbegin();
        while (i != data.coords().cend()) {
            if (i->key() == key) {return &*i;}
            i++;
        }
        return 0;
    }

    static bool compare_coord(const internal::ServiceQueryCoordinate &c1, const internal::ServiceQueryCoordinate &c2) {
        return c1.key() < c2.key();
    }

    void prepare() {
        if (!sorted) {
            std::sort(data.coords().begin(), data.coords().end(), &compare_coord);
            sorted = true;
        }
    }
};

}

#endif // SONICSOCKET_SERVICEQUERY_H
