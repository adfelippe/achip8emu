cmake_minimum_required(VERSION 3.14)
project(achip8emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Include
include_directories(../src)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})


add_executable(${CMAKE_PROJECT_NAME} 
               ../src/main.cpp
               ../src/Chip8.cpp
               ../src/SdlDisplay.cpp
               ../src/SdlKeyboard.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} 
                      ${SDL2_LIBRARIES}
                      ${CURSES_LIBRARIES})
