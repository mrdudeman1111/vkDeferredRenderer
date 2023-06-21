CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Trolling BB")

file(GLOB SOURCES
  ${CMAKE_SOURCE_DIR}/*.cpp)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

add_executable(game ${SOURCES})

target_link_libraries(game vulkan glfw glm assimp VulkanMemoryAllocator)

