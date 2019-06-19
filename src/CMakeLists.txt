project (Particles)

find_package(Vulkan)

#preprocessor definitions
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_FORCE_RADIANS)

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
    "${SOURCE_DIRECTORY}/Lighting.cpp"
)

# directories to look for libs
set(EXTERNAL_LIB_DIRECTORIES 
    "${CMAKE_BINARY_DIR}/external/glfw/src"
)

set(EXTERNAL_LIBS 
    ${Vulkan_LIBRARY} 
    glfw3
)

include_directories(Particles ${ALL_INCLUDE_DIRECTORIES})
link_directories(Particles ${EXTERNAL_LIB_DIRECTORIES})

add_executable(Particles "${SOURCE_FILES}")

target_link_libraries(Particles ${EXTERNAL_LIBS} )

#define runtime resource directory
set(RESOURCE_DIRECTORIES
    "resources/"
)

#post build, copy runtime resources to directory
foreach(RESOURCE_DIRECTORY ${RESOURCE_DIRECTORIES})
    add_custom_command(
        TARGET Particles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/${RESOURCE_DIRECTORY}"
            "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_DIRECTORY}"
    )
endforeach()