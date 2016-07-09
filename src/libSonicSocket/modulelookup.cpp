#include "modulelookup.h"

namespace sonic_socket
{

ModuleLookup::ModuleLookup()
{
}

void ModuleLookup::set_interface(const std::string &str)
{
    assert(!str.empty());

    request.set_interface(str);
}

void ModuleLookup::set_version(unsigned int version)
{
    assert(version > 0);

    request.set_version(version);
}

void ModuleLookup::push_prop(const std::string &key, const std::string &val, float weight)
{
    add_match_entry(key, weight)->set_string_value(val);
}

void ModuleLookup::push_prop(const std::string &key, float val, float weight)
{
    add_match_entry(key, weight)->set_scalar_value(val);
}

std::string ModuleLookup::to_string() const
{
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

ModuleSpaceCoordinate *ModuleLookup::add_match_entry(const std::string &key, float weight)
{
    assert(!key.empty());

    ModuleSpaceCoordinate *match = request.add_coords();
    match->set_key(key);
    match->set_weight(weight);
    return match;
}

}
