#include "RetransmissionRequestSender.h"
#include "Receiver.h"
#include <thread>

using std::chrono::duration_cast;
using std::chrono::system_clock;

void RetransmissionRequestSender::run(uint64_t  id) {
    while (true) {
        if (id != validRunID)
            break;

        stateMutex.lock();
        if (!requestsToSend.empty()) {
            auto request = requestsToSend.front();
            requestsToSend.pop_front();
            stateMutex.unlock();

            std::this_thread::sleep_for(request.first - system_clock::now());
            //Send here LOUDER_PLS
        } else {
            isValidRun = false;
            stateMutex.unlock();
            break;
        }
    }
}
