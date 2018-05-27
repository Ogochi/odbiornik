#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <sstream>
#include "StationsFetcher.h"
#include "err.h"

using std::thread;
using std::istringstream;

StationsFetcher::~StationsFetcher() {
    close(sock);
}

void StationsFetcher::setUpSocket() {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        syserr("socket");

    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
        syserr("setsockopt broadcast");

//    optval = TTL_VALUE;
//    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0)
//        syserr("setsockopt multicast ttl");

    const char *remoteDottedAddr = receiver->DISCOVER_ADDR.c_str();
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_port = htons(receiver->CTRL_PORT);
    if (inet_aton(remoteDottedAddr, &remoteAddress.sin_addr) == 0)
        syserr("inet_aton");
}

void StationsFetcher::sendLookUpPeriodically(int periodInSeconds) {
    const char *buffer = "ZERO_SEVEN_COME_IN";
    size_t length = strlen(buffer);

    while (true) {
        fetchId++;

        if (sendto(sock, buffer, length, 0, (struct sockaddr *) &remoteAddress, sizeof remoteAddress) != length)
            std::cerr << "Did not send whole buffer!" << std::endl;

        bool removedCurrentStation = false;
        receiver->stationsMutex.lock();
        receiver->stateMutex.lock();
        // We remove station if it did not respond again during last 4 periods
        while(receiver->stations->front()->timestamp == fetchId - 4) {
            if (receiver->state != STATION_NOT_SELECTED &&
                receiver->stations->front()->equals(receiver->currentStation)) {
                    removedCurrentStation = true;
            }
            receiver->stations->pop_front();
        }
        // Handling current station removal
        if (removedCurrentStation) {
            if (!receiver->stations->empty()) {
                receiver->state = STATION_CHANGED;
                receiver->currentStation = receiver->stations->back();
            } else {
                receiver->state = STATION_NOT_SELECTED;
            }
        }
        receiver->stateMutex.unlock();
        receiver->stationsMutex.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(periodInSeconds));
    }
}

void StationsFetcher::listenForReplies() const {
    while(true) {
        char buffer[200];
        ssize_t length;
        struct sockaddr_in transmitterAddr;
        socklen_t transmitterAddrLen = sizeof transmitterAddr;

        length = recvfrom(sock, buffer, 200, 0, (struct sockaddr *) &transmitterAddr, &transmitterAddrLen);

        if (length < 0) {
            std::cerr << "Listener error!" << std::endl;
            continue;
        }

        string msg = string(buffer);
        std::istringstream stm(msg);
        string s;

        // Parsing message
        if ((stm >> s) && s == "BOREWICZ_HERE") {
            string addr, name;
            stm >> addr >> s >> name;
            int port;
            try {
                port = std::stoi(s);
            } catch(std::exception &e) {
                continue;
            }

            while(stm >> s)
                name += s;

            Station *newStation = new Station(addr, transmitterAddr, port, name, fetchId);
            receiver->stationsMutex.lock();
            // Removing the same station with old timestamp
            for (auto i = receiver->stations->begin(); i != receiver->stations->end(); i++) {
                if ((*i)->equals(newStation)) {
                    receiver->stations->erase(i);
                    break;
                }
            }
            receiver->stations->push_back(newStation);
            receiver->stationsMutex.unlock();

            if ((receiver->isPrefferedStationSet && receiver->prefferedStation == name) ||
                !receiver->isPrefferedStationSet) {
                    receiver->stateMutex.lock();
                    // Setting station if there is no selected station
                    if (receiver->state == STATION_NOT_SELECTED) {
                        receiver->currentStation = newStation;
                        if (receiver->isPlaybackRunning) {
                            receiver->state = STATION_CHANGED;
                        } else {
                            receiver->state = STATION_SELECTED;
                            thread t = thread([this](){ receiver->startPlayback(); });
                            t.detach();
                        }
                    }
                    receiver->stateMutex.unlock();
            }
        }
    }
}

void StationsFetcher::run() {
    setUpSocket();

    thread fetcherSend([this](){ sendLookUpPeriodically(5); });
    fetcherSend.detach();
    thread fetcherListen([this](){ listenForReplies(); });
    fetcherListen.detach();
}

