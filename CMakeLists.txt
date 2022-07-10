cmake_minimum_required(VERSION 3.1.6)
project(dnsrelay)

set(CMAKE_C_FLAGS "-std=gnu11")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE SRC_DIR ${PROJECT_SOURCE_DIR}/src/*.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SRC_DIR})

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} Ws2_32)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME} pthread)
ENDIF()


