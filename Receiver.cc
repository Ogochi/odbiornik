#include "Receiver.h"


void Receiver::addFetcher() {
    stationsFetcher = new StationsFetcher(this);
}

void Receiver::run() {
    // DEBUG
    std::cout << DISCOVER_ADDR << " " << DATA_PORT << " " << CTRL_PORT << " " << UI_PORT << " " << BSIZE << " " << RTIME;
    if (isPrefferedStationSet)
        std::cout << " " << prefferedStation << "\n";

    addFetcher();
}

Receiver::~Receiver() {
    delete stationsFetcher;

    while (!stations->empty()) {
        Station *s = stations->front();
        stations->pop();
        delete(s);
    }

    delete stations;
}