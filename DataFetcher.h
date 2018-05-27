#ifndef SIK3_DATAFETCHER_H
#define SIK3_DATAFETCHER_H


#include "Receiver.h"

class Receiver;

class DataFetcher {
    friend class Receiver;
private:
    Receiver *receiver;

    int sock;
    struct sockaddr_in remoteAddress;

    DataFetcher(Receiver *rec) : receiver(rec) {}

    void run();

public:
    DataFetcher() = delete;
};


#endif //SIK3_DATAFETCHER_H
