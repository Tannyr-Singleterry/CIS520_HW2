cmake_minimum_required(VERSION 2.8)

# ALL OF THE SPECIFIED NAMES IN THIS CMAKE ARE REQUIRED.
# Set the project name variable to hw2.
# THIS IS REQUIRED
project(hw2)

# Add your CMAKE_C_FLAGS AND CMAKE_CXX_FLAGS here
# Make SURE to use -std=c11 and -std=c++11 where relevant.
# The options specified below are required, but additional options
#  may be used.
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror")

# Add our include directory to CMake's search paths
# THIS IS REQUIRED
add_library(dyn_array src/dyn_array.c)
target_include_directories(dyn_array PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# Create library from dyn_array so we can use it later
add_library(process_scheduling src/process_scheduling.c)
target_include_directories(process_scheduling PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(process_scheduling PRIVATE dyn_array)

# analysis executable
add_executable(analysis src/analysis.c)
target_include_directories(analysis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(analysis PRIVATE process_scheduling dyn_array)

# test executable
add_executable(${PROJECT_NAME}_test test/tests.cpp)
target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}_test gtest pthread process_scheduling dyn_array)
