cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(Visualizer)

if (MSVC)
    add_custom_target(RERUN_CMAKE del /f generated.stamp COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR})

    function(assign_source_group main_subfolder_name main_group_name)
        foreach(_source IN ITEMS ${ARGN})
            get_filename_component(_source_path "${_source}" PATH)
            string(REPLACE ${main_subfolder_name} "${main_group_name}/" _source_path "${_source_path}")
            string(REPLACE "/" "\\" _source_path "${_source_path}")
            source_group("${_source_path}" FILES "${_source}")
        endforeach()
    endfunction(assign_source_group)
endif()

set(CMAKE_CXX_STANDARD 20)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(GLEW REQUIRED PATHS "extern/glew")
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED  PATHS "extern/glm")
find_package(cxxopts REQUIRED  PATHS "extern/cxxopts")

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "include/*.hpp")

if (MSVC)
    assign_source_group("src" "Source Files" ${SOURCES})
    assign_source_group("include" "Header Files" ${HEADERS})
endif()

add_executable(Visualizer ${SOURCES} ${HEADERS})

target_include_directories(Visualizer PRIVATE ${GLM_INCLUDE_DIRS}
                                              ${CMAKE_CURRENT_SOURCE_DIR}/include
                                              $<TARGET_PROPERTY:libglew_shared,INTERFACE_INCLUDE_DIRECTORIES>
                                              $<TARGET_PROPERTY:cxxopts::cxxopts,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(Visualizer PRIVATE ${GLM_DEFINITIONS} NOMINMAX)

target_compile_options(Visualizer PUBLIC /W4 /WX)

set(shared_precompiled_headers <set> <map> <array> <string> <vector> <memory> <thread> <cstdint> <sstream> <iostream> <algorithm> <string_view> <fstream>)

target_precompile_headers(Visualizer PRIVATE <visualizer.hpp> ${shared_precompiled_headers})

target_link_libraries(Visualizer libglew_shared ${OPENGL_LIBRARIES})

if (MSVC)
set_target_properties(Visualizer PROPERTIES VS_DEBUGGER_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/Visualizer.exe VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/ VS_STARTUP_PROJECT Vislualizer)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Visualizer)

add_custom_command(TARGET Visualizer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${CMAKE_BINARY_DIR}/$<CONFIG>/Visualizer.exe
                   ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/Visualizer.exe)
endif()
