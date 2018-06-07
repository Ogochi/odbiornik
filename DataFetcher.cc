#include "DataFetcher.h"
#include "err.h"
#include <iostream>
#include <thread>

using std::cout;
using std::thread;

void DataFetcher::reset() {
    // We change socket only if mcast or dataport changed
    if (receiver->currentStation->MCAST_ADDR != sockMCAST || receiver->currentStation->DATA_PORT != sockDATAPORT || sock == -1) {
        setupSocket(receiver->currentStation->MCAST_ADDR, receiver->currentStation->DATA_PORT);
    }

    receiver->isPlaybackRunning = true;

    receiver->retransmissionRequestSender->stateMutex.lock();
    receiver->retransmissionRequestSender->requestsToSend.clear();
    receiver->retransmissionRequestSender->stateMutex.unlock();

    dataMutex.lock();
    dataBuffer.clear();
    receivedFirstPackage = false;
    dataMutex.unlock();
}

void DataFetcher::run() {
    receiver->stateMutex.lock();
    reset();
    receiver->stateMutex.unlock();

    ssize_t rcv_len;
    char *buffer = (char*)malloc(1000000 * sizeof(char));
    bool hasToExit = false;

    while(true) {
        receiver->stateMutex.lock();
        switch (receiver->state) {
            case STATION_SELECTED:
                break;
            case STATION_CHANGED:
                isValidPlayback = false;
                validPlaybackID++;
                receiver->state = STATION_SELECTED;
                reset();
                break;
            case STATION_NOT_SELECTED:
                isValidPlayback = false;
                validPlaybackID++;
                close(sock);
                sock = -1;
                receiver->isPlaybackRunning = false;
                hasToExit = true;
                break;
        }
        receiver->stateMutex.unlock();

        if (hasToExit)
            break;

        rcv_len = read(sock, buffer, 1000000);

        if (rcv_len <= 0) {
            continue;
        } else if (rcv_len >= 16) {

            string msg = string(buffer, rcv_len);
            Package p;

            // Parsing package
            p.sessionId = parseStringToUINT64(msg.substr(0, 8));
            p.firstByteNum = parseStringToUINT64(msg.substr(8, 8));
            p.audioData = msg.substr(16, rcv_len - 16);

            dataMutex.lock();

            if (!receivedFirstPackage) {
                receivedFirstPackage = true;
                BYTE0 = p.firstByteNum;
                sessionId = p.sessionId;
            }

            if (sessionId == p.sessionId) {
                // Adding new package
                dataBuffer.insert({p.firstByteNum, p});

                // Adding retransmission requests if added package is the one with the greatest
                // firstByteNum and is not the first package
                if (p.firstByteNum != dataBuffer.begin()->first && p.firstByteNum == dataBuffer.rbegin()->first) {
                    uint64_t missingByteNum = (++dataBuffer.rbegin())->first + p.audioData.size();

                    // If we received too big p.firstByteNum we do not want to request retransmission of too low packs
                    missingByteNum = std::max(missingByteNum,
                            p.firstByteNum - (receiver->BSIZE - (receiver->BSIZE % p.audioData.size())));

                    bool isRetransmissionNeeded = false;
                    string missingPackages;

                    while (missingByteNum < p.firstByteNum) {
                        if (isRetransmissionNeeded) {
                            missingPackages += "," + std::to_string(missingByteNum);
                        } else {
                            isRetransmissionNeeded = true;
                            missingPackages += std::to_string(missingByteNum);
                        }

                        missingByteNum += p.audioData.size();
                    }

                    if (isRetransmissionNeeded) {
                        receiver->retransmissionRequestSender->stateMutex.lock();
                        receiver->retransmissionRequestSender->requestsToSend.push_back(
                                {std::chrono::system_clock::now() + std::chrono::milliseconds(receiver->RTIME),
                                 missingPackages});
                        receiver->retransmissionRequestSender->stateMutex.unlock();
                    }
                }

                // Removing too old packages
                if (!dataBuffer.empty()) {
                    auto mapIter = dataBuffer.begin();
                    bool shouldErase = false;

                    while (mapIter->first != dataBuffer.end()->first &&
                           (int64_t)mapIter->first < (int64_t)p.firstByteNum + (rcv_len - 16) - receiver->BSIZE) {
                        mapIter++;
                        shouldErase = true;
                    }

                    if (shouldErase)
                        dataBuffer.erase(dataBuffer.begin(), mapIter);
                }

                // Starting playback if buffer is filled enough
                if (!isValidPlayback && p.firstByteNum >= BYTE0 + (receiver->BSIZE / 4) * 3) {
                    isValidPlayback = true;
                    thread t = thread([this]() { startPlayback(BYTE0, validPlaybackID); });
                    t.detach();
                }
                dataMutex.unlock();
            } else if (sessionId < p.sessionId) {
                // Reset
                isValidPlayback = false;
                validPlaybackID++;
                dataMutex.unlock();
                receiver->stateMutex.lock();
                reset();
                receiver->stateMutex.unlock();
            } else {
                // Ignoring package with lesser sessionId
                dataMutex.unlock();
            }
        }
    }

    free(buffer);
}

void DataFetcher::startPlayback(uint64_t nextFirstByteNum, uint64_t playbackId) {
    while (true) {
        // Check if this playback is valid
        if (validPlaybackID != playbackId)
            break;

        auto mapIter = dataBuffer.find(nextFirstByteNum);
        if (mapIter == dataBuffer.end()) {
            // Needed package is not present
            isValidPlayback = false;
            validPlaybackID++;
            dataMutex.unlock();
            receiver->stateMutex.lock();
            reset();
            receiver->stateMutex.unlock();
            break;
        } else {
            // Putting audio to STDOUT
            cout << mapIter->second.audioData;
            nextFirstByteNum += mapIter->second.audioData.size();
        }
    }
}

void DataFetcher::setupSocket(string multicastDottedAddress, int dataPort) {
    socketMutex.lock();
    if (sock != -1)
        close(sock);

    sockMCAST = multicastDottedAddress;
    sockDATAPORT = dataPort;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        syserr("socket");

    // Bind to local address and port
    struct sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(dataPort);
    if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0)
        syserr("bind");

    // Bind to multicast
    struct ip_mreq ip_mreq;
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(multicastDottedAddress.c_str(), &ip_mreq.imr_multiaddr) == 0)
        syserr("inet_aton");
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof ip_mreq) < 0)
        syserr("setsockopt");

    // Adding timeout
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        syserr("socketopt(RCVTIMEO)");
    socketMutex.unlock();
}

// Parses string(bytes of unit64 in network order) to uint64
uint64_t DataFetcher::parseStringToUINT64(string s) {
    uint64_t result = 0;
    for (int i = 0; i < 8; i++) {
        result += ((uint64_t)(unsigned char)(s[i])) << (8*(8 - i - 1));
    }

    return result;
}
