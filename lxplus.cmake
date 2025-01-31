
cmake_minimum_required(VERSION 3.2)
message(STATUS "Cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
project(atlas-grid-simulator)
# Disable annoying warnings
add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
set(CMAKE_CXX_STANDARD 17)


# Find Boost
find_package(Boost REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX )

# Add json subdir
add_subdirectory(json)
list( APPEND BUILT_PACKAGES "json")

# include directories
include_directories(${Boost_INCLUDE_DIR} ${HDF5_CXX_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/json/single_include)
include_directories(include/)
include_directories(plugins/include/)
include_directories(simgrid/include/ simgrid/build/include/)
# source files
file(GLOB_RECURSE SOURCE_FILES
    "src/*.cpp"
    "util/*.cpp"
    "plugins/src/*.cpp"
)


# generating the executable
add_executable(atlas-grid-simulator ${SOURCE_FILES})

target_link_libraries(atlas-grid-simulator
  "${CMAKE_SOURCE_DIR}/simgrid/build/lib/libsimgrid.so"
	    ${Boost_LIBRARIES}
	    ${HDF5_CXX_LIBRARIES}
            )

