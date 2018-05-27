#ifndef SIK3_STATION_H
#define SIK3_STATION_H

#include <string>
#include <arpa/inet.h>

using std::string;

class Station {
public:
    string MCAST_ADDR;
    struct sockaddr_in transmitterAddr;
    int DATA_PORT;
    string name;
    long long timestamp;

    Station() = delete;
    Station(string mcastAddr, struct sockaddr_in transAddr, int port, string &n, long long ts) :
            MCAST_ADDR(mcastAddr), transmitterAddr(transAddr), DATA_PORT(port), name(n), timestamp(ts) {}

    bool equals(Station *station) {
        return this->MCAST_ADDR == station->MCAST_ADDR && this->DATA_PORT == station->DATA_PORT &&
               this->name == station->name;
    }
};

#endif //SIK3_STATION_H
