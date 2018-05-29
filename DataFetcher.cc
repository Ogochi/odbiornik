#include "DataFetcher.h"
#include "err.h"


void DataFetcher::run() {
//    setupSocket();


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
}
