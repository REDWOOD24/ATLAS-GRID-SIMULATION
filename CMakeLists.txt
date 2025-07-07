cmake_minimum_required(VERSION 3.12)

message(STATUS "Cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
project(SimATLAS VERSION 1.0)

# Disable annoying warnings
add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

# Find SimGrid, Boost, and HDF5

find_package(Boost REQUIRED)
find_package(SimGrid REQUIRED)
find_package (SQLite3 REQUIRED)
find_package(FSMod REQUIRED)
find_package(spdlog REQUIRED)

# Add json subdir
add_subdirectory(json)
list(APPEND BUILT_PACKAGES "json")

# Include directories
include_directories(${SimGrid_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${FSMOD_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/json/single_include)
include_directories(include/)

# Source files
file(GLOB_RECURSE SOURCE_FILES
		"src/*.cpp"
		"util/*.cpp"
)

# Generating the executable
add_executable(atlas-grid-simulator ${SOURCE_FILES}
		include/job_executor.h
		util/job_executor.cpp)

target_link_libraries(atlas-grid-simulator
		${SimGrid_LIBRARY}
		${Boost_LIBRARIES}
		${FSMOD_LIBRARY}
		SQLite::SQLite3
		spdlog::spdlog
)

# ==============================
# Installation Configuration
# ==============================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install header files
install(FILES include/job.h include/DispatcherPlugin.h include/host_extensions.h
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SimATLAS)

# Install the executable
#install(TARGETS atlas-grid-simulator
#        DESTINATION ${CMAKE_INSTALL_BINDIR})

# Create a package configuration file
configure_package_config_file(
"${CMAKE_SOURCE_DIR}/cmake/SimATLASConfig.cmake.in"
"${CMAKE_BINARY_DIR}/SimATLASConfig.cmake"
INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/SimATLAS"
)

install(FILES "${CMAKE_BINARY_DIR}/SimATLASConfig.cmake"
DESTINATION ${CMAKE_INSTALL_DATADIR}/SimATLAS)

# Generate a version file
write_basic_package_version_file(
"${CMAKE_BINARY_DIR}/SimATLASConfigVersion.cmake"
VERSION ${PROJECT_VERSION}
COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_BINARY_DIR}/SimATLASConfigVersion.cmake"
DESTINATION ${CMAKE_INSTALL_DATADIR}/SimATLAS)