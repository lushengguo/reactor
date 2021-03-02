#include "../INetAddr.hpp"
#include <iostream>
using namespace reactor;

static bool INetAdddr_test()
{
    std::string ip   = "192.168.0.1";
    uint16_t    port = 13;
    INetAddr    addr(ip, port);

    return addr.ip() == ip && addr.port() == port;
}

int main() { std::cout << std::boolalpha << INetAdddr_test() << std::endl; }