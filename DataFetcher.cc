#include "DataFetcher.h"
#include "err.h"
#include <iostream>
#include <thread>

using std::cout;
using std::thread;

void DataFetcher::reset() {
    // mozna sprawdzic czy cokolwiek sie zmienilo zamiast na pale robic nowy socket
    setupSocket(receiver->currentStation->MCAST_ADDR, receiver->currentStation->DATA_PORT);
    receiver->isPlaybackRunning = true;

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
            case STATION_CHANGED:
                isValidPlayback = false;
                validPlaybackID++;
                close(sock);
                reset();
                break;
            case STATION_NOT_SELECTED:
                isValidPlayback = false;
                validPlaybackID++;
                close(sock);
                receiver->isPlaybackRunning = false;
                hasToExit = true;
                break;
        }
        receiver->stateMutex.unlock();

        if (hasToExit)
            break;

        rcv_len = read(sock, buffer, sizeof buffer);
        if (rcv_len < 0) {
            std::cerr << "Data Fetcher read nothing" << std::endl;
            continue;
        } else {
            auto p = reinterpret_cast<Package*>(buffer);
            dataMutex.lock();
            // Case when received first package
            if (!receivedFirstPackage) {
                receivedFirstPackage = true;
                BYTE0 = p->firstByteNum;
                sessionId = p->sessionId;
            }

            if (sessionId == p->sessionId) {
                // Removing too old packages
                auto mapIter = dataBuffer.begin();
                bool shouldErase = false;
                while (mapIter->first < p->firstByteNum + strlen(p->audioData) - receiver->BSIZE) {
                    mapIter++;
                    shouldErase = true;
                }
                if (shouldErase)
                    dataBuffer.erase(dataBuffer.begin(), --mapIter);
                // Adding new package
                dataBuffer[p->firstByteNum] = *p;
                // Starting playback if buffer is filled enough
                if (!isValidPlayback && p->firstByteNum >= BYTE0 + receiver->BSIZE * 3 / 4) {
                    isValidPlayback = true;
                    thread([this]() { startPlayback(BYTE0, validPlaybackID); });
                }
            } else if (sessionId < p->sessionId) {
                isValidPlayback = false;
                validPlaybackID++;
                close(sock);
                reset();
            }
            dataMutex.unlock();
        }
    }

    free(buffer);
}

void DataFetcher::startPlayback(uint64_t nextFirstByteNum, uint64_t playbackId) {
    while (true) {
        // Check if this playback is valid
        if (validPlaybackID != playbackId)
            break;

        dataMutex.lock();
        auto mapIter = dataBuffer.find(nextFirstByteNum);
        if (mapIter == dataBuffer.end()) {
            // Needed package is not present
            receiver->stateMutex.lock();
            receiver->state = STATION_CHANGED;
            receiver->stateMutex.unlock();
            dataMutex.unlock();
            break;
        } else {
            cout << mapIter->second.audioData;
            nextFirstByteNum += strlen(mapIter->second.audioData);
        }
        dataMutex.unlock();
    }
}

void DataFetcher::setupSocket(string multicastDottedAddress, int dataPort) {
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

}
