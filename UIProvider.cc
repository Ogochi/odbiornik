#include "UIProvider.h"


void UIProvider::setUpSocket() {
    for (int i = 0; i < (int)maxClients; i++) {
        clients[i].fd = -1;
        clients[i].events = POLLIN;
        clients[i].revents = 0;
    }


    clients[0].fd = socket(PF_INET, SOCK_STREAM, 0);
    if (clients[0].fd < 0) {
        perror("Opening stream socket");
        exit(EXIT_FAILURE);
    }

    // Make reusable
    int iSetOption = 1;
    setsockopt(clients[0].fd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

    // Bind to local address and port
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(receiver->UI_PORT);
    if (bind(clients[0].fd, (struct sockaddr*)&server,
             (socklen_t)sizeof(server)) < 0) {
        perror("Binding stream socket");
        exit(EXIT_FAILURE);
    }
}

void UIProvider::run() {
    std::cerr << "UI running!\n";
    setUpSocket();

    char buffer[1];
    int activeClients = 0;

    // Welcoming clients
    if (listen(clients[0].fd, 5) == -1) {
        perror("Starting to listen");
        exit(EXIT_FAILURE);
    }

    // Handling client connections
    while (true) {
        for (int i = 0; i < (int)maxClients; ++i)
            clients[i].revents = 0;

        // Waiting for 100 ms
        int ret = poll(clients, maxClients, 100);

        if (ret < 0)
            perror("poll");
        else if (ret >= 0) {
            if (clients[0].revents & POLLIN) {
                std::cerr << "POLLIN\n";
                int msgSock = accept(clients[0].fd, (struct sockaddr*)0, (socklen_t*)0);

                if (msgSock == -1)
                    perror("accept");
                else {
                    for (int i = 1; i < (int)maxClients; ++i) {
                        if (clients[i].fd == -1) {
                            std::cerr << "Adding client\n";
                            clientsMutex.lock();
                            clients[i].fd = msgSock;
                            clientsMutex.unlock();
                            clientInput[i].reset();
                            activeClients++;

                            sendTelnetConfig(msgSock);
                            clearClientTerminal(msgSock);
                            sendMenu(msgSock);
                            break;
                        }
                    }
                }
            }
            for (int i = 1; i < (int)maxClients; ++i) {
                if (clients[i].fd != -1 && (clients[i].revents & POLLIN)) {
                    ssize_t rval = read(clients[i].fd, buffer, 1);
                    if (rval <= 0) {
                        std::cerr << "Ending connection" << std::endl;
                        if (close(clients[i].fd) < 0)
                            perror("close");
                        clientsMutex.lock();
                        clients[i].fd = -1;
                        clientsMutex.unlock();
                        activeClients--;
                    } else {
                        std::cerr << "Read sth\n";
                        clientInput[i].putInput(buffer[0]);

                        if (clientInput[i].isArrowUp()) {
                            changeStation(-1);
                            sendEveryoneNewMenu();
                        } else if (clientInput[i].isArrowDown()) {
                            changeStation(1);
                            sendEveryoneNewMenu();
                        }
                    }
                }
            }
        }
    }
}

void UIProvider::changeStation(int move) {
    std::cerr << "Telent changing station" << std::endl;
    receiver->stateMutex.lock();
    receiver->stationsMutex.lock();
    for (auto stationsIter = receiver->stations->begin(); stationsIter != receiver->stations->end(); stationsIter++) {
        if ((*stationsIter)->equals(receiver->currentStation)) {
            // Two corner cases and two general
            if ((*stationsIter)->equals(*receiver->stations->begin()) && move == -1)
                receiver->currentStation = *receiver->stations->rbegin();
            else if ((*stationsIter)->equals(*receiver->stations->rbegin()) && move == 1)
                receiver->currentStation = *receiver->stations->begin();
            else if (move == -1)
                receiver->currentStation = *(--stationsIter);
            else if (move == 1)
                receiver->currentStation = *(++stationsIter);

            receiver->state = STATION_CHANGED;
            break;
        }
    }
    receiver->stationsMutex.unlock();
    receiver->stateMutex.unlock();
}

bool UIProvider::sendTelnetConfig(int socket) {
    // IAC DO LINEMODE
    char config[3] = {(char)255, (char)253, (char)34};
    if (write(socket, config, 3) != 3)
        return false;
    // IAC SB LINEMODE MODE 0 IAC SE
    char config2[7] = {(char)255, (char)250, (char)34, (char)1, (char)0, (char)255, (char)240};
    if (write(socket, config2, 7) != 7)
        return false;
    // IAC WILL ECHO
    char config3[3] = {(char)255, (char)251, (char)1};
    if (write(socket, config3, 3) != 3)
        return false;
    return true;
}

bool UIProvider::clearClientTerminal(int socket) {
    // codes for terminal clear and cursor move to (0, 0)
    char clear[32] = "\033[2J\033[0;0H";
    return write(socket, clear, 32) == 32;
}

bool UIProvider::sendMenu(int socket) {
    bool result = true;
    string line = "------------------------------------------------------------------------\r\n";
    string header = "  SIK Radio\r\n";

    // Sending header
    ssize_t sent = write(socket, line.c_str(), line.size());
    sent += write(socket, header.c_str(), header.size());
    sent += write(socket, line.c_str(), line.size());
    if ((uint64_t)sent != 2 * line.size() + header.size())
        result = false;

    // Sending menu
    receiver->stateMutex.lock();
    receiver->stationsMutex.lock();
    for (auto station : *(receiver->stations)) {
        string menuLine = "  ";
        if (station->equals(receiver->currentStation))
            menuLine += "> ";
        else
            menuLine += "  ";

        menuLine += station->name + "\r\n";
        if ((int64_t)menuLine.size() != write(socket, menuLine.c_str(), menuLine.size()))
            result = false;
    }
    receiver->stationsMutex.unlock();
    receiver->stateMutex.unlock();

    // Sending bottom line
    if ((int64_t)line.size() != write(socket, line.c_str(), line.size()))
        result = false;
    return result;
}

void UIProvider::sendEveryoneNewMenu() {
    std::cerr << "Sending new menu everyone\n";
    for (int i = 1; i < (int)maxClients; ++i) {
        if (clients[i].fd != -1) {
            std::cerr << "A\n";
            clearClientTerminal(clients[i].fd);
            sendMenu(clients[i].fd);
        }
    }
}

UIProvider::~UIProvider() {
    close(clients[0].fd);
}

void InputAutomaton::putInput(char in) {
    switch ((int)in) {
        case (int)'\033':
            state = 1;
            break;
        case (int)'[':
            if (state == 1)
                state = 2;
            else
                state = 0;
            break;
        case (int)'A':
            if (state == 2)
                state = 3;
            else
                state = 0;
            break;
        case (int)'B':
            if (state == 2)
                state = 4;
            else
                state = 0;
            break;
    }
    std::cerr << "Curr state: " << state << std::endl;
}