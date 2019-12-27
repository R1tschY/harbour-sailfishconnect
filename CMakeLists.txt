cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0071 NEW)
project(SailfishConnect VERSION 0.7 LANGUAGES C CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_CXX_STANDARD 14)
set(SAILFISHOS ON)
set(BUILD_SHARED_LIBS ON)

# Set RPATH
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/harbour-sailfishconnect/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)
set(CMAKE_INSTALL_RPATH "${LIB_INSTALL_DIR}")

add_subdirectory(app)
add_subdirectory(kdeconnect)
#add_subdirectory(tests)

# Install libraries
install(
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/deps/lib"
  DESTINATION "${LIB_INSTALL_DIR}/.."
  FILES_MATCHING PATTERN "libKF5*.so.5"
)
