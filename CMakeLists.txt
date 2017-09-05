cmake_minimum_required(VERSION 3.8)
project(AquaBlock)

add_subdirectory(./src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

find_library(SDL_FRAMEWORK SDL2)

if (NOT SDL_FRAMEWORK)
    message(FATAL_ERROR "SDL2 not found")
endif()

add_executable(${PROJECT_NAME} ${SOURCEFILES})

target_link_libraries(${PROJECT_NAME}
    ${SDL_FRAMEWORK}
)
