cmake_minimum_required(VERSION 3.10)
project(sik3)

find_package(Boost COMPONENTS program_options REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

set(CMAKE_CXX_STANDARD 17)

add_executable(sik3 main.cc Receiver.cc Receiver.h ReceiverBuilder.h ReceiverBuilder.cc)

target_link_libraries(sik3 ${Boost_LIBRARIES})