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
    ModuleLookup();

    void set_interface(const std::string &str);
    void set_version(unsigned int version);
    void push_prop(const std::string &key, const std::string &val, float weight = 1.0f);
    void push_prop(const std::string &key, float val, float weight = 1.0f);

    ModuleLookupRequest &get_request() {return request;}

    std::string to_string() const;

private:
    ModuleLookupRequest request;

    ModuleSpaceCoordinate *add_match_entry(const std::string &key, float weight);
};

}

#endif // SONICSOCKET_MODULELOOKUP_H
