#ifndef SIK3_RECEIVER_H
#define SIK3_RECEIVER_H


#include "ReceiverBuilder.h"
#include <string>
#include <iostream>

using std::string;

class Receiver {
    friend class ReceiverBuilder;
private:
    string DISCOVER_ADDR, prefferedStation;
    bool isPrefferedStationSet = false;
    int DATA_PORT, CTRL_PORT, UI_PORT, BSIZE, RTIME;

    Receiver(string _DISCOVER_ADDR, int _DATA_PORT, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME) :
            DISCOVER_ADDR(_DISCOVER_ADDR), DATA_PORT(_DATA_PORT), CTRL_PORT(_CTRL_PORT), UI_PORT(_UI_PORT), BSIZE(_BSIZE),
            RTIME(_RTIME) {};
    Receiver(string _DISCOVER_ADDR, int _DATA_PORT, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME,
             string _prefferedStation) : DISCOVER_ADDR(_DISCOVER_ADDR), DATA_PORT(_DATA_PORT), CTRL_PORT(_CTRL_PORT),
                                         UI_PORT(_UI_PORT), BSIZE(_BSIZE), RTIME(_RTIME), isPrefferedStationSet(true),
                                         prefferedStation(_prefferedStation) {};
public:
    Receiver() = delete;

    void run();
};


#endif //SIK3_RECEIVER_H
