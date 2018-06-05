#include "RetransmissionRequestSender.h"
#include "Receiver.h"
#include <thread>
#include <sstream>

using std::chrono::duration_cast;
using std::chrono::system_clock;

void RetransmissionRequestSender::run() {
    while (true) {
        stateMutex.lock();
        if (!requestsToSend.empty()) {
            std::pair<time_point<system_clock>, string> request = requestsToSend.front();
            requestsToSend.pop_front();
            stateMutex.unlock();

//            std::cerr << "Sleeping for: " <<std::chrono::duration_cast<std::chrono::milliseconds>(
//                    request.first - system_clock::now()).count() << std::endl;
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(
                    request.first - system_clock::now()));
            std::istringstream packNums(request.second);
            string pack;
            string requestString;
            bool isRequestEmpty = true;

            while (std::getline(packNums, pack, ',')) {
                if (pack.size() > 8)
                    std::cerr << "Stoll err: " << request.second << "\n";
//                std::cerr << "to stoll: " << pack <<std::endl;
                int64_t num = std::stoll(pack);

                receiver->dataFetcher->dataMutex.lock();
//                std::cerr << num << " vs " << receiver->dataFetcher->BYTE0 << std::endl;
                if (num < receiver->dataFetcher->BYTE0 || num < receiver->dataFetcher->dataBuffer.begin()->first) {
                    receiver->dataFetcher->dataMutex.unlock();
                    std::cerr << "Not retransmitting!\n";
                    break;
                }
                if (receiver->dataFetcher->dataBuffer.find(num) == receiver->dataFetcher->dataBuffer.end()) {
//                    std::cerr << "considerable request\n";
                    receiver->dataFetcher->dataMutex.unlock();
                    if (isRequestEmpty) {
                        isRequestEmpty = false;
                        requestString += std::to_string(num);
                    } else {
                        requestString += "," + std::to_string(num);
                    }
                } else {
                    receiver->dataFetcher->dataMutex.unlock();
                }
            }

            if (!isRequestEmpty) {
//                std::cerr << "request not empty, sending\n";
                stateMutex.lock();
                requestsToSend.push_back(
                        {std::chrono::system_clock::now() + std::chrono::milliseconds(receiver->RTIME),
                                         requestString});
                stateMutex.unlock();

                requestString = "LOUDER_PLEASE " + requestString + "\n";
                receiver->dataFetcher->socketMutex.lock();
//                std::cerr << "Sent retransmission request!\n";
                sendto(receiver->dataFetcher->sock, requestString.c_str(), requestString.size(), 0,
                       (struct sockaddr *) &receiver->currentStation->transmitterAddr,
                               sizeof receiver->currentStation->transmitterAddr);
                receiver->dataFetcher->socketMutex.unlock();
            }
        } else {
            stateMutex.unlock();
        }
    }
}
