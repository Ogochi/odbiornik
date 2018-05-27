#ifndef SIK3_RECEIVER_H
#define SIK3_RECEIVER_H


#include "ReceiverBuilder.h"
#include "Station.h"
#include "StationsFetcher.h"
#include <string>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>

using std::string;
using std::thread;
using std::queue;
using std::mutex;

class StationsFetcher;

class Receiver {
    friend class ReceiverBuilder;
    friend class StationsFetcher;
private:
    string DISCOVER_ADDR, prefferedStation;
    bool isPrefferedStationSet = false;
    int DATA_PORT, CTRL_PORT, UI_PORT, BSIZE, RTIME;
    StationsFetcher *stationsFetcher;

    Receiver(string _DISCOVER_ADDR, int _DATA_PORT, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME) :
            DISCOVER_ADDR(_DISCOVER_ADDR), DATA_PORT(_DATA_PORT), CTRL_PORT(_CTRL_PORT), UI_PORT(_UI_PORT), BSIZE(_BSIZE),
            RTIME(_RTIME) {};
    Receiver(string _DISCOVER_ADDR, int _DATA_PORT, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME,
             string _prefferedStation) : DISCOVER_ADDR(_DISCOVER_ADDR), DATA_PORT(_DATA_PORT), CTRL_PORT(_CTRL_PORT),
                                         UI_PORT(_UI_PORT), BSIZE(_BSIZE), RTIME(_RTIME), isPrefferedStationSet(true),
                                         prefferedStation(_prefferedStation) {};


    queue<Station*> *stations = new queue<Station*>();
    mutex stationsMutex;

    void addFetcher();
public:
    Receiver() = delete;
    ~Receiver();

    void run();
};


#endif //SIK3_RECEIVER_H
