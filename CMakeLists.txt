
project(AirflowNetwork)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(dependencies/pugixml-1.9)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/pugixml-1.9/src)
add_subdirectory(src)
add_subdirectory(test)
