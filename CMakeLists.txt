
project(AirflowNetwork)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(dependencies/pugixml-1.9)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/pugixml-1.9/src)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/skyline/include)
add_subdirectory(src)
add_subdirectory(test)
