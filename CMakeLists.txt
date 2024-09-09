
cmake_minimum_required(VERSION 3.2)

# lxplus option (OFF by default)
option(lxplus "Enable lxplus-specific configuration" OFF)

# If lxplus is enabled, skip the rest of the script
if(lxplus)
    include(lxplus.cmake)
    return()
endif()

message(STATUS "Cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
project(atlas-grid-simulator)
# Disable annoying warnings
add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")


# Find SimGrid and Boost
find_package(SimGrid REQUIRED)
find_package(Boost REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX )


# Add json subdir
add_subdirectory(json)
list( APPEND BUILT_PACKAGES "json")

# include directories
include_directories(${SimGrid_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${HDF5_CXX_INCLUDE_DIRS})
include_directories(include/)
include_directories(plugins/include/)

# source files
file(GLOB_RECURSE SOURCE_FILES
    "src/*.cpp"
    "util/*.cpp"
    "plugins/src/*.cpp"
)


# generating the executable
add_executable(atlas-grid-simulator ${SOURCE_FILES})

target_link_libraries(atlas-grid-simulator
            ${SimGrid_LIBRARY}
	    ${Boost_LIBRARIES}
	    ${HDF5_CXX_LIBRARIES}
            )

