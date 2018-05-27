#ifndef SIK3_RECEIVER_H
#define SIK3_RECEIVER_H


#include "ReceiverBuilder.h"
#include "Station.h"
#include "StationsFetcher.h"
#include "DataFetcher.h"
#include <string>
#include <iostream>
#include <list>
#include <thread>
#include <mutex>

using std::string;
using std::thread;
using std::list;
using std::mutex;

class StationsFetcher;
class DataFetcher;

enum State {
    STATION_NOT_SELECTED,
    STATION_SELECTED,
    STATION_CHANGED,
};

class Receiver {
    friend class ReceiverBuilder;
    friend class StationsFetcher;
    friend class DataFetcher;
private:
    string DISCOVER_ADDR, prefferedStation;
    bool isPrefferedStationSet = false;
    int DATA_PORT, CTRL_PORT, UI_PORT, BSIZE, RTIME;
    StationsFetcher *stationsFetcher;
    DataFetcher *dataFetcher;

    Receiver(string _DISCOVER_ADDR, int _DATA_PORT, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME) :
            DISCOVER_ADDR(_DISCOVER_ADDR), DATA_PORT(_DATA_PORT), CTRL_PORT(_CTRL_PORT), UI_PORT(_UI_PORT), BSIZE(_BSIZE),
            RTIME(_RTIME) {};
    Receiver(string _DISCOVER_ADDR, int _DATA_PORT, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME,
             string _prefferedStation) : DISCOVER_ADDR(_DISCOVER_ADDR), DATA_PORT(_DATA_PORT), CTRL_PORT(_CTRL_PORT),
                                         UI_PORT(_UI_PORT), BSIZE(_BSIZE), RTIME(_RTIME), isPrefferedStationSet(true),
                                         prefferedStation(_prefferedStation) {};


    list<Station*> *stations = new list<Station*>();
    mutex stationsMutex; // Guards 'stations'

    Station *currentStation;
    State state = STATION_NOT_SELECTED;
    bool isPlaybackRunning = false;
    mutex stateMutex; // Guards 'currentStation', 'state' and 'isPlaybackRunning'

    void setupFetchers();
    void startPlayback();
public:
    Receiver() = delete;
    ~Receiver();

    void run();
};


#endif //SIK3_RECEIVER_H
