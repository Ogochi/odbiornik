#include "Receiver.h"


void Receiver::setupComponents() {
    dataFetcher = new DataFetcher(this);
    stationsFetcher = new StationsFetcher(this);
    retransmissionRequestSender = new RetransmissionRequestSender(this);
    uiProvider = new UIProvider(this);

    thread t = thread([this](){ uiProvider->run(); });
    t.detach();

    t = thread([this](){ stationsFetcher->run(); });
    t.detach();

    retransmissionRequestSender->run();
}

void Receiver::run() {
    if (isPrefferedStationSet)
        std::cout << " " << prefferedStation << "\n";

    setupComponents();
}

void Receiver::startFetchingData() {
    dataFetcher->run();
}

Receiver::~Receiver() {
    delete stationsFetcher;
    delete dataFetcher;
    delete retransmissionRequestSender;
    delete uiProvider;

    while (!stations->empty()) {
        Stations *s = stations->front();
        stations->pop_front();
        delete(s);
    }

    delete stations;
}