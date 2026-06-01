cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0071 NEW)
project(SailfishConnect VERSION ${PACKAGE_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
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

# Compile position independent code and export whats needed
# Needed by booster
set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN hidden)

# AUTOMOC
set(CMAKE_AUTOMOC_DEPEND_FILTERS
  "K_PLUGIN_CLASS_WITH_JSON" "[\n][ \t]*K_PLUGIN_CLASS_WITH_JSON[ \t]*\\(\\w+,[ \t]*\"([^\"]+)\""
)

# compile definitions
add_definitions(-DQT_USE_QSTRINGBUILDER)
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-DQT_NO_DEBUG_OUTPUT -DQT_NO_DEBUG)
endif()
add_definitions(-DSAILFISHOS)

# add dependencies
include(cmake/CPM.cmake)

function(AddKF5Package)
    set(options )
    set(oneValueArgs NAME VERSION)
    set(multiValueArgs PATCHES)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    string(TOLOWER "${arg_NAME}" LOWERCASE_NAME)
    string(REGEX MATCH "^[0-9]+\\.[0-9]+" SHORT_VERSION "${arg_VERSION}")
    CPMAddPackage(
        NAME "KF5${arg_NAME}"
        URL "https://download.kde.org/stable/frameworks/${SHORT_VERSION}/k${LOWERCASE_NAME}-${arg_VERSION}.zip"
        VERSION "${arg_VERSION}"
        PATCHES "${arg_PATCHES}"
        EXCLUDE_FROM_ALL ON
        SYSTEM ON
        OPTIONS "BUILD_QCH OFF" "BUILD_TESTING OFF"
    )
    file(WRITE
        ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/kf5${LOWERCASE_NAME}-config-version.cmake
        "include(\"${${CPM_LAST_PACKAGE_NAME}_BINARY_DIR}/${CPM_LAST_PACKAGE_NAME}ConfigVersion.cmake\")\n")
endfunction()

AddKF5Package(
    NAME CoreAddons VERSION 5.36.0
    PATCHES "cmake/kcoreaddons.patch"
)

AddKF5Package(
    NAME I18n VERSION 5.36.0
    PATCHES "cmake/ki18n.patch"
)

AddKF5Package(
    NAME Config VERSION 5.36.0
    PATCHES "cmake/kconfig.patch"
)

set(LIBSSH_VERSION 0.11.0)
string(REGEX MATCH "^[0-9]+\\.[0-9]+" LIBSSH_SHORT_VERSION "${LIBSSH_VERSION}")
CPMAddPackage(
  NAME libssh
  URL "https://www.libssh.org/files/${LIBSSH_SHORT_VERSION}/libssh-${LIBSSH_VERSION}.tar.xz"
  VERSION ${LIBSSH_VERSION}
  EXCLUDE_FROM_ALL ON
  SYSTEM ON
  OPTIONS
    "WITH_ZLIB ON"
    "WITH_SFTP ON"
    "WITH_SERVER ON"
    "BUILD_STATIC_LIB ON"
    "UNIT_TESTING OFF"
    "WITH_EXAMPLES OFF"
    "WITH_PCAP OFF"
    "WITH_SYMBOL_VERSIONING OFF"
    "WITH_ABI_BREAK ON"
)
if(libssh_ADDED)
  target_compile_definitions(ssh-static INTERFACE LIBSSH_STATIC=1 WITH_SERVER=1)
endif()

# Sailfish Connect
find_package(ECM 5.99 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMQtDeclareLoggingCategory)

# KDE Connect
add_subdirectory(kdeconnect-kde)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Network Gui Sql Quick)
find_package(KF5 REQUIRED COMPONENTS I18n CoreAddons Config)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
remove_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_FOREACH
)

# Options
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
