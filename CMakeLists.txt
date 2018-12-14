cmake_minimum_required (VERSION 3.8)
project (vulkan_minimal_compute)

find_package(Vulkan)

# get rid of annoying MSVC warnings.
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set (CMAKE_CXX_STANDARD 11)

set(EXTERNAL_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/")

set(INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(ALL_INCLUDE_DIRECTORIES
	"${INCLUDE_DIRECTORY}"
	"${Vulkan_INCLUDE_DIR}"
    "${EXTERNAL_INCLUDE_DIRECTORY}"
)


set (SRC_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (SRC_FILES
	"${SRC_DIRECTORY}/main.cpp"
    "${SRC_DIRECTORY}/ComputeApplication.cpp"
)

set(ALL_LIBS ${Vulkan_LIBRARY} )

include_directories(${ALL_INCLUDE_DIRECTORIES})

add_executable(vulkan_minimal_compute "${SRC_FILES}")

set_target_properties(vulkan_minimal_compute PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_link_libraries(vulkan_minimal_compute ${ALL_LIBS} )


set(RESOURCE_DIRECTORIES
    "resources/"
)

#post build, copy runtime resources to directory
foreach(RESOURCE_DIRECTORY ${RESOURCE_DIRECTORIES})
    add_custom_command(
        TARGET vulkan_minimal_compute POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_DIRECTORY}"
            "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_DIRECTORY}"
    )
endforeach()