#include "Receiver.h"


void Receiver::setupFetchers() {
    dataFetcher = new DataFetcher(this);
    stationsFetcher = new StationsFetcher(this);
    stationsFetcher->run();
}

void Receiver::run() {
    // DEBUG
    std::cout << DISCOVER_ADDR << " " << CTRL_PORT << " " << UI_PORT << " " << BSIZE << " " << RTIME;
    if (isPrefferedStationSet)
        std::cout << " " << prefferedStation << "\n";

    setupFetchers();
}

void Receiver::startFetchingData() {
    dataFetcher->run();
}

Receiver::~Receiver() {
    delete stationsFetcher;
    delete dataFetcher;

    while (!stations->empty()) {
        Stations *s = stations->front();
        stations->pop_front();
        delete(s);
    }

    delete stations;
}