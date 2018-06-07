#include "UIProvider.h"


void UIProvider::setUpSocket() {
    for (int i = 0; i < maxClients; i++) {
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
    size_t length;
    ssize_t rval;
    int activeClients;

    // Welcoming clients
    if (listen(clients[0].fd, 5) == -1) {
        perror("Starting to listen");
        exit(EXIT_FAILURE);
    }
    std::cerr << "After welcoming clients\n";

    // Handling client connections
    while (true) {
        for (auto client : clients)
            client.revents = 0;

        // Waiting for 100 ms
        int ret = poll(clients, maxClients, 1000);

        if (ret < 0)
            perror("poll");
        else if (ret >= 0) {
            if (clients[0].revents & POLLIN) {
                std::cerr << "POLLIN\n";
                int msgSock = accept(clients[0].fd, (struct sockaddr*)0, (socklen_t*)0);

                if (msgSock == -1)
                    perror("accept");
                else {
                    for (int i = 1; i < maxClients; ++i) {
                        if (clients[i].fd == -1) {
                            std::cerr << "Adding client\n";
                            clients[i].fd = msgSock;
                            activeClients++;
                            break;
                        }
                    }
                }
            }
            for (int i = 1; i < maxClients; ++i) {
                if (clients[i].fd != -1 && (clients[i].revents & POLLIN)) {
                    rval = read(clients[i].fd, buffer, 1);
                    if (rval <= 0) {
                        std::cerr << "Ending connection" << std::endl;
                        if (close(clients[i].fd) < 0)
                            perror("close");
                        clients[i].fd = -1;
                        activeClients--;
                    } else {
                        std::cerr << "Read sth\n";
                        printf("-->%.*s\n", (int) rval, buffer);
                    }
                }
            }
        }
    }
}

UIProvider::~UIProvider() {
    close(clients[0].fd);
}