
cmake_minimum_required(VERSION 3.2)
message(STATUS "Cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
project(atlas-grid-simulator)
# Disable annoying warnings
add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")


# Find  SimGrid
#find_package(WRENCH REQUIRED)
find_package(SimGrid REQUIRED)
find_package(Boost REQUIRED)
add_subdirectory(json)
list( APPEND BUILT_PACKAGES "json")

# include directories
include_directories(${SimGrid_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
include_directories(include/)

# source files
set(SOURCE_FILES
        src/main.cpp
	util/parser.cpp
	util/platform.cpp
	util/actors.cpp
	util/data.cpp
	util/storage.cpp
        )

# generating the executable
add_executable(atlas-grid-simulator ${SOURCE_FILES})

target_link_libraries(atlas-grid-simulator
            ${SimGrid_LIBRARY}
	    ${Boost_LIBRARIES}
            )

