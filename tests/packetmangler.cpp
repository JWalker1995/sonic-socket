#include "catch/single_include/catch.hpp"
#include "libSonicSocket/packetmangler.h"

const char *orig = "Testing PacketMangler<DataType>... Hopefully this works!";

TEST_CASE("PacketMangler<unsigned char>", "")
{
    unsigned char *mut = new unsigned char[56];
    std::copy_n(orig, 56, reinterpret_cast<char *>(mut));

    sonic_socket::PacketMangler<unsigned char>::mangle<100>(mut, 0, 1);
    sonic_socket::PacketMangler<unsigned char>::demangle<100>(mut, 0, 1);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned char>::mangle<100>(mut, 0, 2);
    sonic_socket::PacketMangler<unsigned char>::demangle<100>(mut, 0, 2);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned char>::mangle<100>(mut, 0, 3);
    sonic_socket::PacketMangler<unsigned char>::demangle<100>(mut, 0, 3);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned char>::mangle<100>(mut, 0, 4);
    sonic_socket::PacketMangler<unsigned char>::demangle<100>(mut, 0, 4);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned char>::mangle<100>(mut, 0, 56);
    sonic_socket::PacketMangler<unsigned char>::demangle<100>(mut, 0, 56);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));
}

TEST_CASE("PacketMangler<unsigned int>", "")
{
    unsigned int *mut = new unsigned int[56];
    std::copy_n(orig, 56, reinterpret_cast<char *>(mut));

    sonic_socket::PacketMangler<unsigned int>::mangle<100>(mut, 0, 1);
    sonic_socket::PacketMangler<unsigned int>::demangle<100>(mut, 0, 1);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned int>::mangle<100>(mut, 0, 2);
    sonic_socket::PacketMangler<unsigned int>::demangle<100>(mut, 0, 2);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned int>::mangle<100>(mut, 0, 3);
    sonic_socket::PacketMangler<unsigned int>::demangle<100>(mut, 0, 3);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned int>::mangle<100>(mut, 0, 4);
    sonic_socket::PacketMangler<unsigned int>::demangle<100>(mut, 0, 4);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned int>::mangle<100>(mut, 0, 56);
    sonic_socket::PacketMangler<unsigned int>::demangle<100>(mut, 0, 56);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));
}

TEST_CASE("PacketMangler<unsigned long long int>", "")
{
    unsigned long long int *mut = new unsigned long long int[56];
    std::copy_n(orig, 56, reinterpret_cast<char *>(mut));

    sonic_socket::PacketMangler<unsigned long long int>::mangle<100>(mut, 0, 1);
    sonic_socket::PacketMangler<unsigned long long int>::demangle<100>(mut, 0, 1);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned long long int>::mangle<100>(mut, 0, 2);
    sonic_socket::PacketMangler<unsigned long long int>::demangle<100>(mut, 0, 2);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned long long int>::mangle<100>(mut, 0, 3);
    sonic_socket::PacketMangler<unsigned long long int>::demangle<100>(mut, 0, 3);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned long long int>::mangle<100>(mut, 0, 4);
    sonic_socket::PacketMangler<unsigned long long int>::demangle<100>(mut, 0, 4);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));

    sonic_socket::PacketMangler<unsigned long long int>::mangle<100>(mut, 0, 56);
    sonic_socket::PacketMangler<unsigned long long int>::demangle<100>(mut, 0, 56);
    REQUIRE(std::equal(orig, orig + 56, reinterpret_cast<char *>(mut)));
}
