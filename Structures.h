#ifndef SIK3_STRUCTURES_H
#define SIK3_STRUCTURES_H

#include <string>
#include <arpa/inet.h>

using std::string;

class Stations {
public:
    string MCAST_ADDR;
    struct sockaddr_in transmitterAddr;
    int DATA_PORT;
    string name;
    long long timestamp;

    Stations() = delete;
    Stations(string mcastAddr, struct sockaddr_in transAddr, int port, string &n, long long ts) :
            MCAST_ADDR(mcastAddr), transmitterAddr(transAddr), DATA_PORT(port), name(n), timestamp(ts) {}

    bool equals(Stations *station) {
        return this->MCAST_ADDR == station->MCAST_ADDR && this->DATA_PORT == station->DATA_PORT &&
               this->name == station->name;
    }
};

struct Package {
    uint64_t sessionId;
    uint64_t firstByteNum;
    char *audioData;
};

#endif //SIK3_STRUCTURES_H
