cmake_minimum_required(VERSION 3.14)
project(lora_gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(gateway src/main.cpp src/LoadBalancer.cpp src/GatewayController.cpp)

# MacOS specific: httplib might need these frameworks
if(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
    target_link_libraries(gateway PRIVATE ${COREFOUNDATION_LIBRARY} ${SECURITY_LIBRARY})
endif()

# If we need threads (httplib uses them)
find_package(Threads REQUIRED)
target_link_libraries(gateway PRIVATE Threads::Threads)
