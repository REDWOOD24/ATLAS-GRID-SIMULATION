cmake_minimum_required(VERSION 3.12)

message(STATUS "Cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
project(SimATLAS VERSION 1.1)

# Disable annoying warnings
add_definitions(-DSPDLOG_FMT_EXTERNAL)
add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find required packages
find_package(Boost     REQUIRED)
find_package(SimGrid   REQUIRED)
find_package(SQLite3   REQUIRED)
find_package(FSMod     REQUIRED)
find_package(spdlog    REQUIRED)

# Add json subdir
add_subdirectory(json)
list(APPEND BUILT_PACKAGES "json")

# Include directories
include_directories(${SimGrid_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${FSMOD_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/json/single_include)
include_directories(include/)

# Source files
file(GLOB_RECURSE SOURCE_FILES
#        "src/*.cpp"
        "util/*.cpp"
)

# Build the library
add_library(SimATLAS ${SOURCE_FILES})

# Generating the executable
add_executable(atlas-grid-simulator src/main.cpp)

target_link_libraries(atlas-grid-simulator
        SimATLAS
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


# Install Library
install(TARGETS SimATLAS
    EXPORT SimATLASTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install header files
install(FILES include/job.h include/DispatcherPlugin.h include/host_extensions.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SimATLAS)

# Install the executable
install(TARGETS atlas-grid-simulator
        DESTINATION ${CMAKE_INSTALL_BINDIR})

# Export targets to a file
install(EXPORT SimATLASTargets
	NAMESPACE SimATLAS::
    	FILE SimATLASTargets.cmake
    	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/SimATLAS
)

# Create a package configuration file
configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/cmake/SimATLASConfig.cmake.in"
        "${CMAKE_BINARY_DIR}/SimATLASConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake/SimATLAS"
)

install(FILES "${CMAKE_BINARY_DIR}/SimATLASConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/SimATLAS)

# Generate a version file
write_basic_package_version_file(
        "${CMAKE_BINARY_DIR}/SimATLASConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_BINARY_DIR}/SimATLASConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/SimATLAS)

