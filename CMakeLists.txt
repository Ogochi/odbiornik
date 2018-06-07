cmake_minimum_required(VERSION 3.8)
project(odbiornik)

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(Threads REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()
if(NOT Threads_FOUND)
    message(FATAL_ERROR "Could not find threads!")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall")

# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} ${CMAKE_THREAD_LIBS_INIT} )
    endif()
endmacro()

add_executable(odbiornik main.cc Receiver.cc Receiver.h ReceiverBuilder.h ReceiverBuilder.cc Structures.h StationsFetcher.cc StationsFetcher.h err.cc err.h DataFetcher.cc DataFetcher.h RetransmissionRequestSender.cc RetransmissionRequestSender.h UIProvider.cc UIProvider.h)

target_link_libraries(odbiornik ${Boost_LIBRARIES})

install(TARGETS DESTINATION .)
