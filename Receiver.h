#ifndef SIK3_RECEIVER_H
#define SIK3_RECEIVER_H


#include "ReceiverBuilder.h"
#include "Structures.h"
#include "StationsFetcher.h"
#include "DataFetcher.h"
#include "RetransmissionRequestSender.h"
#include <string>
#include <iostream>
#include <list>
#include <thread>
#include <mutex>

using std::string;
using std::thread;
using std::list;
using std::mutex;

class ReceiverBuilder;
class StationsFetcher;
class DataFetcher;
class RetransmissionRequestSender;

enum State {
    STATION_NOT_SELECTED,
    STATION_SELECTED,
    STATION_CHANGED,
};

class Receiver {
    friend class ReceiverBuilder;
    friend class StationsFetcher;
    friend class DataFetcher;
    friend RetransmissionRequestSender;
private:
    string DISCOVER_ADDR, prefferedStation;
    bool isPrefferedStationSet = false;
    int CTRL_PORT, UI_PORT, BSIZE, RTIME;
    StationsFetcher *stationsFetcher;
    DataFetcher *dataFetcher;
    RetransmissionRequestSender *retransmissionRequestSender;

    Receiver(string _DISCOVER_ADDR, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME) :
            DISCOVER_ADDR(_DISCOVER_ADDR), CTRL_PORT(_CTRL_PORT), UI_PORT(_UI_PORT), BSIZE(_BSIZE),
            RTIME(_RTIME) {};
    Receiver(string _DISCOVER_ADDR, int _CTRL_PORT, int _UI_PORT, int _BSIZE, int _RTIME, string _prefferedStation) :
            DISCOVER_ADDR(_DISCOVER_ADDR), CTRL_PORT(_CTRL_PORT), UI_PORT(_UI_PORT), BSIZE(_BSIZE), RTIME(_RTIME),
            isPrefferedStationSet(true), prefferedStation(_prefferedStation) {};


    list<Stations*> *stations = new list<Stations*>();
    mutex stationsMutex; // Guards 'stations'

    Stations *currentStation;
    State state = STATION_NOT_SELECTED;
    bool isPlaybackRunning = false;
    mutex stateMutex; // Guards 'currentStation', 'state' and 'isPlaybackRunning'

    void setupFetchers();
    void startFetchingData();
public:
    Receiver() = delete;
    ~Receiver();

    void run();
};


#endif //SIK3_RECEIVER_H
