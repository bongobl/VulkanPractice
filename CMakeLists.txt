cmake_minimum_required (VERSION 3.8)

#build glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
add_subdirectory(external/glfw)

#build BasicVulkanRender
add_subdirectory(src bin)