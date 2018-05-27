#include "Receiver.h"


void Receiver::setupFetchers() {
    dataFetcher = new DataFetcher(this);
    stationsFetcher = new StationsFetcher(this);
    stationsFetcher->run();
}

void Receiver::run() {
    // DEBUG
    std::cout << DISCOVER_ADDR << " " << DATA_PORT << " " << CTRL_PORT << " " << UI_PORT << " " << BSIZE << " " << RTIME;
    if (isPrefferedStationSet)
        std::cout << " " << prefferedStation << "\n";

    setupFetchers();
}

void Receiver::startPlayback() {
    dataFetcher->run();
}

Receiver::~Receiver() {
    delete stationsFetcher;
    delete dataFetcher;

    while (!stations->empty()) {
        Station *s = stations->front();
        stations->pop_front();
        delete(s);
    }

    delete stations;
}