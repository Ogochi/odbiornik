#include "Receiver.h"

void Receiver::run() {
    // DEBUG
    std::cout << DISCOVER_ADDR << " " << DATA_PORT << " " << CTRL_PORT << " " << UI_PORT << " " << BSIZE << " " << RTIME;
    if (isPrefferedStationSet)
        std::cout << " " << prefferedStation << "\n";
}