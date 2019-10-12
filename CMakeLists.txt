cmake_minimum_required(VERSION 3.11)
cmake_policy(SET CMP0071 NEW)
project(SailfishConnect VERSION 0.5 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})
add_definitions(-DQT_STATICPLUGIN)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(app)
add_subdirectory(kdeconnect)
#add_subdirectory(tests)