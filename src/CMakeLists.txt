project(executable)

set(srcs properties.cpp
         node.cpp
		 model.cpp
		 link.cpp
		 powerlaw.cpp)

set(hdrs properties.hpp
         node.hpp
		 model.hpp
		 link.hpp
		 airflowelement.hpp
		 powerlaw.hpp)

set(includes ../dependencies/skyline/include/skyline.hpp)

add_executable(airflownetwork  ${hdrs} ${srcs} ${includes} airflownetwork.cpp)
target_link_libraries(airflownetwork pugixml)
