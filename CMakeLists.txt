cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(VERSION 3.1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include("cmake/defaults.cmake")
set(NAME m3d)

set(CMAKE_CXX_STANDARD 11)

project(${NAME})

add_custom_target(SetupRelease ALL ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(SetupRelease PROPERTIES FOLDER "CMakeTargets")
add_custom_target(SetupDebug ALL ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin_debug)
set_target_properties(SetupDebug PROPERTIES FOLDER "CMakeTargets")

find_package(Vulkan REQUIRED)
link_libraries(${VULKAN_LIBRARY})
include_directories(${VULKAN_INCLUDE_DIR})

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
else()
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    find_package(XCB REQUIRED)
    link_libraries(${XCB_LIBRARIES})
endif()

add_dependency_external_projects(glm)
list(APPEND EXTERNALS glm)
include_directories(${GLM_INCLUDE_DIRS})

add_dependency_external_projects(gli)
list(APPEND EXTERNALS gli)
include_directories(${GLI_INCLUDE_DIRS})

add_dependency_external_projects(glslang)
list(APPEND EXTERNALS glslang)
include_directories(${GLSLANG_INCLUDE_DIRS})
link_libraries(${GLSLANG_LIBRARIES})

add_dependency_external_projects(glog)
list(APPEND EXTERNALS glog)
include_directories(${GLOG_INCLUDE_DIRS})

# add Math lib
add_subdirectory(Math)
link_libraries(Math)
include_directories(Math)

# Vulkan Utils
add_subdirectory(VkUtils)
link_libraries(VkUtils)
include_directories(VkUtils)

# GUI
add_subdirectory(Gui)
link_libraries(Gui)
include_directories(Gui)

foreach(EXTERNAL ${EXTERNALS})
    add_dependencies(VkUtils ${EXTERNAL})
endforeach()

if (NOT WIN32)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif()

add_subdirectory(examples)

