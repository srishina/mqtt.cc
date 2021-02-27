cmake_minimum_required(VERSION 3.11.0)
project(mqttcpp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LIB_CODEC_SOURCES
    lib/packet/codec.cc
    lib/packet/properties.cc
    lib/packet/packet.cc
    lib/packet/connect.cc
    lib/packet/connack.cc
    lib/packet/publish.cc
    lib/packet/subscribe.cc
    lib/packet/suback.cc
    lib/packet/unsubscribe.cc
    lib/packet/unsuback.cc
    lib/packet/publishresponse.cc
)

set(LIB_SOURCES 
    lib/mqtt.cc    
    lib/topic.cc        
    lib/error.cc)
set(LIB_INCLUDES
    include/mqtt/mqtt.h
    include/mqtt/noncopyable.h
    include/mqtt/error.h)
add_library(mqttcpp STATIC ${LIB_CODEC_SOURCES} ${LIB_SOURCES} ${LIB_INCLUDES})
target_include_directories(mqttcpp INTERFACE include PRIVATE include)
# target_include_directories(mqttcpp PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(mqttcpp PRIVATE -Wall -Wextra -Werror -Wshadow -Wdouble-promotion)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    target_compile_options(mqttcpp PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-switch-enum)
endif()
target_compile_options(mqttcpp PRIVATE "-Wformat=2" -Wundef -fno-common -Wconversion)

# todo.. add additional flags for Debugability & Code Size -g3, -Os etc..

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(TEST_CODEC_SOURCES
    lib/packet/codec.test.cc
    lib/packet/properties.test.cc
    lib/packet/connect.test.cc
    lib/packet/connack.test.cc
    lib/packet/publish.test.cc
    lib/packet/subscribe.test.cc
    lib/packet/suback.test.cc
    lib/packet/unsubscribe.test.cc
    lib/packet/unsuback.test.cc
    lib/packet/publishresponse.test.cc)
    
# Make test executable
set(TEST_SOURCES
    doctest/doctest.cpp
    lib/topic.test.cc)
add_executable(mqtt_unit_tests ${TEST_CODEC_SOURCES} ${TEST_SOURCES})
target_compile_options(mqtt_unit_tests PRIVATE -Wall -Wextra -Werror -Wshadow -Wdouble-promotion)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    target_compile_options(mqtt_unit_tests PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded)
endif()
target_compile_options(mqtt_unit_tests PRIVATE "-Wformat=2" -Wundef -fno-common -Wconversion)

target_include_directories(mqtt_unit_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(mqtt_unit_tests PRIVATE mqttcpp)
