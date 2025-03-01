cmake_minimum_required(VERSION 3.15)
project(mypkg CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

message("Project dir: " ${CMAKE_SOURCE_DIR})
message("Binary dir: " ${CMAKE_BINARY_DIR})

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake" STRING "Conan toolchain file")
include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)
# include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)

# Find ncurses package
find_package(Curses REQUIRED)
find_package(GTest REQUIRED)
# ...
# include_directories(${CURSES_INCLUDE_DIR})

message("Curses libraries: " ${Curses_LIBRARIES})


# Add the executable
add_executable(mypkg src/mypkg.cpp src/main.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_DIR="${CMAKE_SOURCE_DIR}"
)

# Link ncurses 
target_link_libraries(mypkg 
    PRIVATE
    Curses::Curses
    gtest::gtest
)

# Enable testing and add test
enable_testing()
add_test(NAME MyTests COMMAND mypkg)

# Set static linking
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     LINK_SEARCH_START_STATIC ON
#     LINK_SEARCH_END_STATIC ON
# )
# target_link_options(${PROJECT_NAME} PRIVATE -static)


# Install?
# install(TARGETS mypkg DESTINATION "."
#         RUNTIME DESTINATION bin
#         ARCHIVE DESTINATION lib
#         LIBRARY DESTINATION lib
#         )


