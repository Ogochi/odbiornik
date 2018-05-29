#include "ReceiverBuilder.h"

Receiver* ReceiverBuilder::build() const {
    if (isPrefferedStationSet)
        return new Receiver(DISCOVER_ADDR, CTRL_PORT, UI_PORT, BSIZE, RTIME, prefferedStation);
    else
        return new Receiver(DISCOVER_ADDR, CTRL_PORT, UI_PORT, BSIZE, RTIME);
}

ReceiverBuilder* ReceiverBuilder::setPrefferedStation(string station) {
    prefferedStation = station;
    isPrefferedStationSet = true;
    return this;
}