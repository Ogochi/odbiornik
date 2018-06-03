#ifndef SIK3_STATIONSFETCHER_H
#define SIK3_STATIONSFETCHER_H


#include "Structures.h"
#include "Receiver.h"
#include <queue>
#include <netinet/in.h>
#include <mutex>

using std::queue;
using std::mutex;

class Receiver;
class RetransmissionRequestSender;

class StationsFetcher {
    friend class Receiver;
    friend class RetransmissionRequestSender;
private:
    Receiver *receiver;

    int sock;
    struct sockaddr_in remoteAddress;

    long long fetchId = 0;

    StationsFetcher(Receiver *rec) : receiver(rec) {}
    ~StationsFetcher();
    void setUpSocket();
    void sendLookUpPeriodically(int periodInSeconds);
    void listenForReplies() const;
    void run();

public:
    StationsFetcher() = delete;
};


#endif //SIK3_STATIONSFETCHER_H
