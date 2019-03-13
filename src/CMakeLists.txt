project (BasicVulkanRender)

find_package(Vulkan)

# get rid of annoying MSVC warnings.
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set (CMAKE_CXX_STANDARD 17)

# directories to include
set(EXTERNAL_INCLUDE_DIRECTORY "${CMAKE_SOURCE_DIR}/external/include")
set(PROJECT_INCLUDE_DIRECTORY "${CMAKE_SOURCE_DIR}/include")
set(ALL_INCLUDE_DIRECTORIES
	"${PROJECT_INCLUDE_DIRECTORY}"
    "${EXTERNAL_INCLUDE_DIRECTORY}"
	"${Vulkan_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/external/glfw/include"  
    "${EXTERNAL_INCLUDE_DIRECTORY}/glm"
)

# source files to compile
set (SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set (SOURCE_FILES
	"${SOURCE_DIRECTORY}/main.cpp"
    "${SOURCE_DIRECTORY}/RenderApplication.cpp"
    "${SOURCE_DIRECTORY}/Utils.cpp"
    "${SOURCE_DIRECTORY}/Vertex.cpp"
    "${SOURCE_DIRECTORY}/QueueFamilyMap.cpp"
    "${SOURCE_DIRECTORY}/SwapChain.cpp"
)

# directories to look for libs
set(EXTERNAL_LIB_DIRECTORIES 
    "${CMAKE_BINARY_DIR}/external/glfw/src"
)

set(EXTERNAL_LIBS 
    ${Vulkan_LIBRARY} 
    glfw3
)

include_directories(BasicVulkanRender ${ALL_INCLUDE_DIRECTORIES})
link_directories(BasicVulkanRender ${EXTERNAL_LIB_DIRECTORIES})

add_executable(BasicVulkanRender "${SOURCE_FILES}")

target_link_libraries(BasicVulkanRender ${EXTERNAL_LIBS} )

#define runtime resource directory
set(RESOURCE_DIRECTORIES
    "resources/"
)

#post build, copy runtime resources to directory
foreach(RESOURCE_DIRECTORY ${RESOURCE_DIRECTORIES})
    add_custom_command(
        TARGET BasicVulkanRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/${RESOURCE_DIRECTORY}"
            "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_DIRECTORY}"
    )
endforeach()