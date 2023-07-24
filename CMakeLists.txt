CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Trolling BB")

set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCES
  ${CMAKE_SOURCE_DIR}/*.cpp)

include_directories(${CMAKE_SOURCE_DIR} /usr/include)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

add_definitions(-DShaderDir="${CMAKE_SOURCE_DIR}/Shaders/")
add_definitions(-DModelDir="${CMAKE_SOURCE_DIR}/Models/")

add_executable(game ${SOURCES})

target_link_libraries(game vulkan glfw glm assimp)

