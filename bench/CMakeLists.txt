set(CMAKE_CXX_STANDARD 17)

add_executable( benchmark main.cpp )
target_link_libraries( benchmark Boost::boost eumorphic )