cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0071 NEW)
project(SailfishConnect VERSION 0.7 LANGUAGES C CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_CXX_STANDARD 14)
set(SAILFISHOS ON)
set(BUILD_SHARED_LIBS OFF)

# Smaller binary sizes
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

# Set RPATH
set(KDE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/share/harbour-sailfishconnect/bin")
set(KDE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/share/harbour-sailfishconnect/include")
set(KDE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/share/harbour-sailfishconnect/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)
set(CMAKE_INSTALL_RPATH "${LIB_INSTALL_DIR}")

# AUTOMOC
set(CMAKE_AUTOMOC_DEPEND_FILTERS
  "K_PLUGIN_CLASS_WITH_JSON" "[\n][ \t]*K_PLUGIN_CLASS_WITH_JSON[ \t]*\\(\\w+,[ \t]*\"([^\"]+)\""
)

# compile definitions
add_definitions(-DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x050600 -DQT_USE_QSTRINGBUILDER)
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-DQT_NO_DEBUG_OUTPUT -DQT_NO_DEBUG)
endif()
add_definitions(-DSAILFISHOS)

# KDE Connect
add_subdirectory(kdeconnect)

# Sailfish Connect
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5 REQUIRED COMPONENTS Core DBus Network Gui Sql Quick)
find_package(KF5 REQUIRED COMPONENTS I18n CoreAddons Config)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

set(PACKAGE_NAME harbour-sailfishconnect)

add_subdirectory(app)
add_subdirectory(pluginscommon)
add_subdirectory(plugins)
#add_subdirectory(tests)


# I18N
set(LOCALE_INSTALL_DIR share/${PACKAGE_NAME}/locale)
KI18N_INSTALL(${CMAKE_CURRENT_SOURCE_DIR}/po)

# Install libraries
install(
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/deps/lib"
  DESTINATION "${LIB_INSTALL_DIR}/.."
  FILES_MATCHING PATTERN "libKF5*.so.5"
)
