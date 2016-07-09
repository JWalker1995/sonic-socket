#ifndef SONICSOCKET_MODULEINIT_H
#define SONICSOCKET_MODULEINIT_H

#include <string>

namespace sonic_socket
{

class ModuleSpaceCoordinateConstructor : public ModuleSpaceCoordinate
{
    ModuleSpaceCoordinateConstructor(const std::string &key, float value, float weight = 1.0f)
    {
        set_key(key);
        set_scalar_value(value);
        set_weight(weight);
    }

    ModuleSpaceCoordinateConstructor(const std::string &key, const std::string &value, float weight = 1.0f)
    {
        set_key(key);
        set_string_value(value);
        set_weight(weight);
    }
};

}

#endif // SONICSOCKET_MODULEINIT_H
