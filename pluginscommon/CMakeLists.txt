# Qt
find_package(Qt5 COMPONENTS Core Sql REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE sailfishconnectpluginscommon_SOURCES "*.cpp")
add_library(sailfishconnectpluginscommon 
  ${sailfishconnectpluginscommon_SOURCES})

target_link_libraries(sailfishconnectpluginscommon
PUBLIC
  Qt5::Core
  Qt5::Network
  Qt5::Sql
  KF5::CoreAddons
PRIVATE
  KF5::I18n
)