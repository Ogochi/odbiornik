#ifndef SIK3_DATAFETCHER_H
#define SIK3_DATAFETCHER_H


#include "Receiver.h"

class Receiver;

class DataFetcher {
    friend class Receiver;
private:
    Receiver *receiver;

    int sock;

    DataFetcher(Receiver *rec) : receiver(rec) {}

    void setupSocket(string multicastDottedAddress, int dataPort);
    void run();

public:
    DataFetcher() = delete;
};


#endif //SIK3_DATAFETCHER_H
