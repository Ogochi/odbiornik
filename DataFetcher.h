#ifndef SIK3_DATAFETCHER_H
#define SIK3_DATAFETCHER_H


#include "Receiver.h"
#include <map>
#include <mutex>

using std::map;
using std::mutex;

class Receiver;

class DataFetcher {
    friend class Receiver;
private:
    Receiver *receiver;

    int sock = -1;

    map<uint64_t, Package> dataBuffer; // <firstByteNum, referring Package>
    uint64_t bufferUsedSpace = 0;
    uint64_t BYTE0 = 0;
    uint64_t sessionId = 0;
    bool receivedFirstPackage = false;
    mutex dataMutex; // Guards 'dataBuffer'

    uint64_t validPlaybackID = 0;
    bool isValidPlayback = false;

    DataFetcher(Receiver *rec) : receiver(rec) {}

    void setupSocket(string multicastDottedAddress, int dataPort);
    void reset(); // Needs 'receiver->stateMutex' locked
    void startPlayback(uint64_t nextFirstByteNum, uint64_t playbackId);
    void run();

public:
    DataFetcher() = delete;
};


#endif //SIK3_DATAFETCHER_H
