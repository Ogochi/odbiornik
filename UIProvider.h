#ifndef ODBIORNIK_UIPROVIDER_H
#define ODBIORNIK_UIPROVIDER_H


#include <poll.h>
#include <vector>
#include "Receiver.h"

using std::vector;

class Receiver;
class StationsFetcher;

class InputAutomaton {
private:
    int state = 0; // 3 - arrowUp | 4 - arrowDown
public:
    InputAutomaton() = default;
    void putInput(char in);
    bool isArrowUp() { return state == 3; }
    bool isArrowDown() { return state == 4; }
    void reset() { state = 0; }
};

class UIProvider {
    friend class Receiver;
    friend class StationsFetcher;
private:
    Receiver *receiver;

    static const uint64_t maxClients = _POSIX_OPEN_MAX - 7;
    pollfd clients[maxClients];
    InputAutomaton clientInput[maxClients];
    mutex clientsMutex; // Guards 'clients' for changing

    UIProvider(Receiver *rec) : receiver(rec) {
        for (int i = 0; i < (int)maxClients; i++) {
            clients[i].fd = -1;
            clients[i].events = POLLIN;
            clients[i].revents = 0;
        }
    }
    ~UIProvider();

    void setUpSocket();
    bool sendTelnetConfig(int socket);
    bool clearClientTerminal(int socket);
    bool sendMenu(int socket);
    void sendEveryoneNewMenu();
    void changeStation(int move);
    void run();

public:
    UIProvider() = delete;
};




#endif //ODBIORNIK_UIPROVIDER_H
