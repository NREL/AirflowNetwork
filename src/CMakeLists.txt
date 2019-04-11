project(executable)

set(srcs properties.cpp)

set(hdrs properties.hpp
         node.hpp
		 model.hpp
		 link.hpp
		 element.hpp
		 powerlaw.hpp)

set(includes ../dependencies/skyline/include/skyline.hpp)

add_executable(airflownetwork  ${hdrs} ${srcs} ${includes} airflownetwork.cpp)
target_link_libraries(airflownetwork pugixml)
