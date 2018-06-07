#include "boost/program_options.hpp"
#include "ReceiverBuilder.h"

#include <iostream>
#include <string>
#include <thread>

namespace po = boost::program_options;
using std::string;
using std::cerr;

int main(int argc, char **argv) {
    po::options_description desc("Options");
    desc.add_options()
            (",d", po::value<string>(), "DISCOVER_ADDR")
            (",C", po::value<uint16_t >(), "CTRL_PORT")
            (",U", po::value<uint16_t>(), "UI_PORT")
            (",b", po::value<int>(), "BSIZE")
            (",R", po::value<int>(), "RTIME")
            (",n", po::value<string>(), "Name of preffered station");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch(std::exception &e) {
        std::cerr << "Wrong program options!" << std::endl;
        return 1;
    }

    ReceiverBuilder *receiverBuilder = new ReceiverBuilder();

    if (vm.count("-d")) {
        receiverBuilder->setDISCOVER_ADDR(vm["-d"].as<string>());
    }
    if (vm.count("-C")) {
        receiverBuilder->setCTRL_PORT(vm["-C"].as<uint16_t>());
    }
    if (vm.count("-U")) {
        receiverBuilder->setUI_PORT(vm["-U"].as<uint16_t>());
    }
    if (vm.count("-b")) {
        receiverBuilder->setBSIZE(vm["-b"].as<int>());
    }
    if (vm.count("-R")) {
        receiverBuilder->setRTIME(vm["-R"].as<int>());
    }
    if (vm.count("-n")) {
        receiverBuilder->setPrefferedStation(vm["-n"].as<string>());
    }

    Receiver* receiver = receiverBuilder->build();
    delete receiverBuilder;

    receiver->run();

    delete receiver;

    return 0;
}