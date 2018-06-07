#ifndef ODBIORNIK_UIPROVIDER_H
#define ODBIORNIK_UIPROVIDER_H


#include <poll.h>
#include <vector>
#include "Receiver.h"

using std::vector;

class Receiver;

class UIProvider {
    friend class Receiver;
private:
    Receiver *receiver;

    static const uint64_t maxClients = _POSIX_OPEN_MAX - 7;
    pollfd clients[maxClients];

    UIProvider(Receiver *rec) : receiver(rec) {}
    ~UIProvider();
    void setUpSocket();

    void run();

public:
    UIProvider() = delete;
};


#endif //ODBIORNIK_UIPROVIDER_H
