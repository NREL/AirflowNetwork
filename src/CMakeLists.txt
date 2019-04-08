project(executable)

add_executable(airflownetwork node.hpp node.cpp model.hpp model.cpp link.hpp link.cpp airflowelement.hpp powerlaw.hpp powerlaw.cpp properties.hpp properties.cpp airflownetwork.cpp)
target_link_libraries(airflownetwork pugixml)
