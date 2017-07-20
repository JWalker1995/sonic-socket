#ifndef SONICSOCKET_MODULELOOKUP_H
#define SONICSOCKET_MODULELOOKUP_H

#include <string>
#include <vector>

#include "libSonicSocket/jw_util/methodcallback.h"

#include "libSonicSocket/base.pb.h"

namespace sonic_socket
{

class ModuleLookup
{
public:
    ModuleLookup() {
    }

    void set_interface(const std::string &str) {
        assert(!str.empty());

        request.set_interface(str);
    }

    void set_version(unsigned int version) {
        assert(version > 0);

        request.set_version(version);
    }

    void push_prop(const std::string &key, const std::string &val, float weight) {
        add_match_entry(key, weight)->set_string_value(val);
    }

    void push_prop(const std::string &key, float val, float weight) {
        add_match_entry(key, weight)->set_scalar_value(val);
    }

    ModuleLookupRequest &get_request() {
        return request;
    }

    std::string to_string() const {
        std::string res;
        res += "ModuleLookup\n";
        res += "    Interface: " + request.interface() + "\n";

        google::protobuf::RepeatedPtrField<ModuleSpaceCoordinate>::const_iterator i = request.coords().cbegin();
        while (i != request.coords().cend())
        {
            res += "    Coordinate: {";
            res += "key=\"" + i->key() + "\", ";
            res += "weight=" + std::to_string(i->weight()) + ", ";
            res += "string_value=\"" + i->string_value() + "\", ";
            res += "scalar_value=" + std::to_string(i->scalar_value());
            res += "}\n";

            i++;
        }

        return res;
    }

private:
    ModuleLookupRequest request;

    ModuleSpaceCoordinate *add_match_entry(const std::string &key, float weight) {
        assert(!key.empty());

        ModuleSpaceCoordinate *match = request.add_coords();
        match->set_key(key);
        match->set_weight(weight);
        return match;
    }
};

}

#endif // SONICSOCKET_MODULELOOKUP_H
