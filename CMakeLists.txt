cmake_minimum_required (VERSION 3.8)
project (BasicVulkanRender)

find_package(Vulkan)

# get rid of annoying MSVC warnings.
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set (CMAKE_CXX_STANDARD 17)

set(EXTERNAL_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/include")
set(EXTERNAL_LIB_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/lib")
set(INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(ALL_INCLUDE_DIRECTORIES
	"${INCLUDE_DIRECTORY}"
	"${Vulkan_INCLUDE_DIR}"
    "${EXTERNAL_INCLUDE_DIRECTORY}"
    "${EXTERNAL_INCLUDE_DIRECTORY}/glm"
)


set (SRC_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (SRC_FILES
	"${SRC_DIRECTORY}/main.cpp"
    "${SRC_DIRECTORY}/RenderApplication.cpp"
    "${SRC_DIRECTORY}/Utils.cpp"
    "${SRC_DIRECTORY}/Vertex.cpp"
    "${SRC_DIRECTORY}/QueueFamilyMap.cpp"
    "${SRC_DIRECTORY}/SwapChain.cpp"
)

set(ALL_LIBS ${Vulkan_LIBRARY} glfw3)

include_directories(BasicVulkanRender ${ALL_INCLUDE_DIRECTORIES})
link_directories(BasicVulkanRender ${EXTERNAL_LIB_DIRECTORY})

add_executable(BasicVulkanRender "${SRC_FILES}")

target_link_libraries(BasicVulkanRender ${ALL_LIBS} )


set(RESOURCE_DIRECTORIES
    "resources/"
)

#post build, copy runtime resources to directory
foreach(RESOURCE_DIRECTORY ${RESOURCE_DIRECTORIES})
    add_custom_command(
        TARGET BasicVulkanRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_DIRECTORY}"
            "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_DIRECTORY}"
    )
endforeach()