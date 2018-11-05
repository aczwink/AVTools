cmake_minimum_required(VERSION 3.6)
project(AVTools)
find_package(XP-CMake REQUIRED) #we need advanced cmake modules

set(CMAKE_CXX_STANDARD 17)

XPC_AutoBuildType()
XPC_OptForHost()
XPC_SetCompileDefinitions()

find_package(StdPlusPlus REQUIRED)

set(LIBS Std++ Std++Static)

add_subdirectory(src_deprober)
add_subdirectory(src_muxprober)
add_subdirectory(src_player)

add_executable(deprober ${SOURCE_FILES_DEPROBER})
target_link_libraries(deprober ${LIBS})

#add_executable(muxprober ${SOURCE_FILES_MUXPROBER})
#target_link_libraries(muxprober ${LIBS})

add_executable(player ${SOURCE_FILES_PLAYER})
target_link_libraries(player ${LIBS})