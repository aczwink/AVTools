cmake_minimum_required(VERSION 3.6)
project(AVTools)

set(CMAKE_CXX_STANDARD 11)

#debug or release
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG)
else()
    message(FATAL_ERROR "Unknown build type. Must be either 'Debug' or 'Release'")
endif()

find_library(ACSTDLIB_LIBRARY NAMES ACStdLib_d)
find_library(ACSTDLIBSTATIC_LIBRARY NAMES ACStdLibStatic_d)

set(LIBS ${ACSTDLIB_LIBRARY} ${ACSTDLIBSTATIC_LIBRARY})

add_subdirectory(src_deprober)
add_subdirectory(src_muxprober)
add_subdirectory(src_player)

add_executable(deprober ${SOURCE_FILES_DEPROBER})
target_link_libraries(deprober ${LIBS})

add_executable(muxprober ${SOURCE_FILES_MUXPROBER})
target_link_libraries(muxprober ${LIBS})

add_executable(player ${SOURCE_FILES_PLAYER})
target_link_libraries(player ${LIBS})