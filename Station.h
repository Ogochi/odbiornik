#ifndef SIK3_STATION_H
#define SIK3_STATION_H

#include <string>

using std::string;

class Station {
public:
    string MCAST_ADDR;
    int DATA_PORT;
    string name;
    int timestamp;

    Station() = delete;
    Station(string addr, int port, string n, int ts) : MCAST_ADDR(addr), DATA_PORT(port), name(n), timestamp(ts) {}
};

#endif //SIK3_STATION_H
