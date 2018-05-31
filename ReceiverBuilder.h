#ifndef SIK3_RECEIVERBUILDER_H
#define SIK3_RECEIVERBUILDER_H


#include "Receiver.h"
#include <string>

using std::string;

#define ALBUM 382783

class Receiver;

class ReceiverBuilder {
private:
    string DISCOVER_ADDR = "255.255.255.255";
    int CTRL_PORT = 30000 + (ALBUM % 10000);
    int UI_PORT = 10000 + (ALBUM % 10000);
    int BSIZE = 65536;
    int RTIME = 250;

    string prefferedStation;
    bool isPrefferedStationSet = false;

public:
    ReceiverBuilder() = default;

    ReceiverBuilder* setDISCOVER_ADDR(string addr) { DISCOVER_ADDR = addr; return this; }
    ReceiverBuilder* setCTRL_PORT(int port) { CTRL_PORT = port; return this; }
    ReceiverBuilder* setUI_PORT(int port) { UI_PORT = port; return this; }
    ReceiverBuilder* setBSIZE(int bytes) { BSIZE = bytes; return this; }
    ReceiverBuilder* setRTIME(int miliseconds) { RTIME = miliseconds; return this; }
    ReceiverBuilder* setPrefferedStation(string station);

    Receiver* build() const;
};


#endif //SIK3_RECEIVERBUILDER_H
