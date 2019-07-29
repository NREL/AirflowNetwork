project(tests)

add_executable(airflownetwork_tests catch.hpp airflowelement_tests.cpp transport_tests.cpp)
include_directories(../src)
