cmake_minimum_required(VERSION 3.14)
project(achip8emu)

set(CMAKE_CXX_STANDARD 17)

# Include
include_directories(../src)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


add_executable(${CMAKE_PROJECT_NAME} 
               ../src/main.cpp
               ../src/chip8.cpp
               ../src/SdlDisplay.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} 
                      ${SDL2_LIBRARIES})
