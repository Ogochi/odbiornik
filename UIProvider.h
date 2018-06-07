#ifndef ODBIORNIK_UIPROVIDER_H
#define ODBIORNIK_UIPROVIDER_H


#include "Receiver.h"

class Receiver;

class UIProvider {
    friend class Receiver;
private:
    Receiver *receiver;

    int sock = -1;

    UIProvider(Receiver *rec) : receiver(rec) {}
    ~UIProvider();
    void setUpSocket();

    void run();

public:
    UIProvider() = delete;
};


#endif //ODBIORNIK_UIPROVIDER_H
