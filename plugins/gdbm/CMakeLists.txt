cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/plugins/gdbm/CMakeModules")

message(STATUS "CMAKE_MODULE_PATH = " "${CMAKE_MODULE_PATH}")

find_package(GDBM)

if (GDBM_FOUND)

    message(STATUS "Found GDBM installation, will build the GDBM plugin.")
    message(STATUS "Using GDBM_INCLUDE_DIR = " "${GDBM_INCLUDE_DIR}")
    message(STATUS "Using GDBM_LIBRARIES   = " "${GDBM_LIBRARIES}")

    include_directories(${GDBM_INCLUDE_DIR})

    add_module(gdbm NWNXgdbm plugin-gdbm)
    target_link_libraries(gdbm ${GDBM_LIBRARIES})

 	add_executable(gdbm2dump gdbm2dump.cpp)
    target_link_libraries(gdbm2dump ${GDBM_LIBRARIES})

else (GDBM_FOUND)
        message(WARNING "GDBM installation not found, will not build the GDBM plugin!")
endif (GDBM_FOUND)

