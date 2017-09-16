cmake_minimum_required(VERSION 3.8)
project(AquaBlock)

message("*----------------------*")
message("| custom compiler test |")
message("*----------------------*")
if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    message("compiler = apple clang")
    
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        message(FATAL_ERROR "compiler version needs to be 8 or greater for c++17 support")
    endif()

    message("this compiler version is supported")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("compiler = clang")
    
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
        message(FATAL_ERROR "compiler version needs to be 6 or greater for C++17 support")
    endif()
    
    message("this compiler version is supported")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message("compiler = GNU")
   
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
        message(FATAL_ERROR "compiler version needs to be 7 or greater for C++17 support")
    endif()
    
    message("this compiler version is supported")
    
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  message("compiler = Intel")
  message(FATAL_ERROR "code has not been tested on this compiler")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  message("compiler = MSVC")
  message(FATAL_ERROR "this compiler does not support the C++17 features that are required")

endif()
message("*----------------------*")


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
target_link_libraries(${PROJECT_NAME} ${SDL_FRAMEWORK})
