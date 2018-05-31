#ifndef SIK3_RETRANSMISSIONREQUESTSENDER_H
#define SIK3_RETRANSMISSIONREQUESTSENDER_H


#include <zconf.h>
#include <deque>
#include <string>
#include <ctime>
#include <mutex>
#include <chrono>
#include <algorithm>


using std::deque;
using std::string;
using std::chrono::time_point;
using std::chrono::system_clock;
using std::mutex;

class Receiver;

class RetransmissionRequestSender {
    friend Receiver;
private:
    Receiver *receiver;

    u_int64_t validRunID = 0;
    bool isValidRun = false;
    deque<std::pair<time_point<system_clock>, string>> requestsToSend; // pair<whenToSend, packages numbers divided by comma>
    mutex stateMutex; // Guards 'isValidRun', 'validRunID' and 'requestsToSend';

    RetransmissionRequestSender(Receiver *rec) : receiver(rec) {}

    void run(uint64_t  id);
public:
    RetransmissionRequestSender() = delete;
};


#endif //SIK3_RETRANSMISSIONREQUESTSENDER_H
