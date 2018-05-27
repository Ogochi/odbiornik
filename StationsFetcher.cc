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

        // Removing not responding stations
        receiver->stationsMutex.lock();
        while(receiver->stations->front()->timestamp == fetchId - 4)
            receiver->stations->pop();
        receiver->stationsMutex.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(periodInSeconds));
    }
}

void StationsFetcher::listenForReplies() const {
    while(true) {
        char buffer[200];
        ssize_t length;

        length = read(sock, buffer, 200);
        if (length < 0) {
            std::cerr << "Listener error!" << std::endl;
            continue;
        }

        string msg = string(buffer);
        std::istringstream stm(msg);
        string s;

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

            Station *newStation = new Station(addr, port, name, fetchId);
            receiver->stationsMutex.lock();
            receiver->stations->push(newStation);
            receiver->stationsMutex.unlock();
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

